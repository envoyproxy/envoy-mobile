package org.chromium.net.impl;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import io.envoyproxy.envoymobile.engine.EnvoyHTTPStream;

/**
 * Consistency layer above the {@link EnvoyHTTPStream} preventing unwarranted Stream operations
 * after a "cancel" operation. There are no "synchronized" - this is Compare And Swap based logic.
 *
 * <p>This contraption ensures that once a "cancel" operation is invoked, there will be no further
 * operations allowed with the EnvoyHTTPStream - subsequent operations will be ignored silently.
 * However, in the event that that one or more EnvoyHTTPStream operations are currently being
 * executed, the "cancel" operation gets postponed: the last concurrent operation will invoke
 * "cancel" at the end.
 *
 * <p>Instance of this class start with a state of "Busy Starting". This ensure that if a cancel
 * is invoked while the stream is being created, that cancel will be executed only once the stream
 * is completely initialized. Doing otherwise leads to unpredictable outcomes.
 */
final class CancelProofEnvoyStream {

  private static final int CANCEL_BIT = 0x8000;
  private final AtomicInteger mState = new AtomicInteger(1); // Busy starting.
  private volatile EnvoyHTTPStream mStream;                  // Cancel can come from any Thread.

  /** Sets the stream. Can only be invoked once. */
  void setStream(EnvoyHTTPStream stream) {
    assert mStream == null;
    mStream = stream;
    if (cancelNeedsToBeInvoked()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /** Initiates the sending of the request headers if the state permits. */
  void sendHeaders(Map<String, List<String>> envoyRequestHeaders, boolean endStream) {
    if (cancelHasAlreadyBeenInvoked()) {
      return; // Already Cancelled - to late to send something.
    }
    mStream.sendHeaders(envoyRequestHeaders, endStream);
    if (cancelNeedsToBeInvoked()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /** Initiates the sending of one chunk of the request body if the state permits. */
  void sendData(ByteBuffer buffer, boolean finalChunk) {
    if (cancelHasAlreadyBeenInvoked()) {
      return; // Already Cancelled - to late to send something.
    }
    // The Envoy Mobile library only cares about the capacity - must use the correct ByteBuffer
    if (buffer.position() == 0) {
      mStream.sendData(buffer, buffer.remaining(), finalChunk);
    } else {
      ByteBuffer resizedBuffer = ByteBuffer.allocateDirect(buffer.remaining());
      buffer.mark();
      resizedBuffer.put(buffer);
      buffer.reset();
      mStream.sendData(resizedBuffer, finalChunk);
    }
    if (cancelNeedsToBeInvoked()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /** Initiates the reading of one chunk of the the request body if the state permits. */
  void readData(int size) {
    if (cancelHasAlreadyBeenInvoked()) {
      return; // Already Cancelled - to late to read something.
    }
    mStream.readData(size);
    if (cancelNeedsToBeInvoked()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /**
   * Cancels the Stream if the state permits. Will be delayed when an operation is concurrently
   * running. Idempotent and Thread Safe.
   */
  void cancel() {
    while (true) {
      int count = mState.get();
      if ((count & CANCEL_BIT) != 0) {
        return; // Cancel already invoked.
      }
      if (mState.compareAndSet(count, count | CANCEL_BIT)) {
        if (count == 0) {
          mStream.cancel(); // Was not busy with other EM operations - cancel right now.
        }
        return;
      }
    }
  }

  private boolean cancelHasAlreadyBeenInvoked() {
    while (true) {
      int count = mState.get();
      if ((count & CANCEL_BIT) != 0) {
        return true; // Already canceled
      }
      if (mState.compareAndSet(count, count + 1)) {
        return false;
      }
    }
  }

  private boolean cancelNeedsToBeInvoked() {
    return mState.decrementAndGet() == CANCEL_BIT; // True if the count is back to zero and canceled
  }
}

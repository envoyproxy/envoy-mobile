package org.chromium.net.impl;

import androidx.annotation.IntDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.IntUnaryOperator;
import io.envoyproxy.envoymobile.engine.EnvoyHTTPStream;

/**
 * Consistency layer above the {@link EnvoyHTTPStream} preventing unwarranted Stream operations
 * after a "cancel" operation. There are no "synchronized" - this is CAS based logic.
 *
 * <p>This contraption ensures that once a "cancel" operation is invoked, there will be no further
 * operations allowed with the EnvoyHTTPStream - subsequent operations will be ignored silently.
 * However, in the event that that one or more EnvoyHTTPStream operations are currently being
 * executed, the "cancel" operation gets postponed: the last concurrent operation will invoke
 * "cancel" at the end.
 *
 * <p>Instance of this class start with a state of "BUSY_STARTING". This ensure that if a cancel
 * is invoked while the stream is being created, that cancel will be executed only once the stream
 * is completely initialized. Doing otherwise leads to unpredictable outcomes.
 */
final class CancelProofEnvoyStream {

  @IntDef(flag = true, // Note: this is a bitmap - some states are concurrent.
          value = {State.BUSY_STARTING, State.BUSY_SENDING_HEADERS, State.BUSY_READING_DATA,
                   State.BUSY_SENDING_DATA, State.CANCELLED})
  @Retention(RetentionPolicy.SOURCE)
  private @interface State {
    int BUSY_STARTING = 1;
    int BUSY_SENDING_HEADERS = 1 << 1;
    int BUSY_READING_DATA = 1 << 2;
    int BUSY_SENDING_DATA = 1 << 3;
    int CANCELLED = 1 << 4;
  }

  private static final BusyStateUnsetter BUSY_STARTING_UNSETTER =
      new BusyStateUnsetter(State.BUSY_STARTING);

  private static final BusyStateSetter BUSY_SENDING_HEADER_SETTER =
      new BusyStateSetter(State.BUSY_SENDING_HEADERS);
  private static final BusyStateUnsetter BUSY_SENDING_HEADER_UNSETTER =
      new BusyStateUnsetter(State.BUSY_SENDING_HEADERS);

  private static final BusyStateSetter BUSY_SENDING_DATA_SETTER =
      new BusyStateSetter(State.BUSY_SENDING_DATA);
  private static final BusyStateUnsetter BUSY_SENDING_DATA_UNSETTER =
      new BusyStateUnsetter(State.BUSY_SENDING_DATA);

  private static final BusyStateSetter BUSY_READING_DATA_SETTER =
      new BusyStateSetter(State.BUSY_READING_DATA);
  private static final BusyStateUnsetter BUSY_READING_DATA_UNSETTER =
      new BusyStateUnsetter(State.BUSY_READING_DATA);

  private final AtomicInteger mState = new AtomicInteger(State.BUSY_STARTING);
  private volatile EnvoyHTTPStream mStream; // Cancel can come from any Thread.

  /**
   * Sets the stream. Can only be invoked once, and {@link #sendHeaders}, {@link #sendData},
   * {@link #readData} will fail if this method has not been invoked first.
   */
  void setStream(EnvoyHTTPStream stream) {
    mStream = stream;
    if (!unsetBusyStarting()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /**
   * Initiates the sending of the request headers if the state permits.
   */
  void sendHeaders(Map<String, List<String>> envoyRequestHeaders, boolean endStream) {
    if (!setBusySendingHeader()) {
      return; // Already Cancelled - to late to send something.
    }
    mStream.sendHeaders(envoyRequestHeaders, endStream);
    if (!unsetBusySendingHeaders()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /**
   * Initiates the sending of one chunk of the request body if the state permits.
   */
  void sendData(ByteBuffer buffer, boolean finalChunk) {
    if (!setBusySendingData()) {
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
    if (!unsetBusySendingData()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /**
   * Initiates the reading of one chunk of the the request body if the state permits.
   */
  void readData(int size) {
    if (!setBusyReadingData()) {
      return; // Already Cancelled - to late to read something.
    }
    mStream.readData(size);
    if (!unsetBusyReadingData()) {
      mStream.cancel(); // Cancel was called meanwhile, so now this is honored.
    }
  }

  /**
   * Cancels the Stream if the state permits. Will be delayed when an operation is concurrently
   * running. Idempotent and Thread Safe.
   *
   * @return true if "cancel" was/will be executed
   */
  void cancel() {
    @State int originalState;
    @State int newState;
    do {
      originalState = mState.get();
      if ((originalState & State.CANCELLED) != 0) {
        return; // Cancel already invoked.
      }
      newState = originalState | State.CANCELLED;
    } while (!mState.compareAndSet(originalState, newState));
    if (newState == State.CANCELLED) {
      // Was not busy with other EM operations - cancel right now.
      mStream.cancel();
    }
  }

  /**
   * Unsets the busy starting state.
   *
   * @return true if not cancelled
   */
  private boolean unsetBusyStarting() {
    return (mState.updateAndGet(BUSY_STARTING_UNSETTER) & State.CANCELLED) != State.CANCELLED;
  }

  /**
   * Sets the busy sending header state if not already cancelled.
   *
   * @return true if not already cancelled
   */
  private boolean setBusySendingHeader() {
    return (mState.updateAndGet(BUSY_SENDING_HEADER_SETTER) & State.CANCELLED) == 0;
  }

  /**
   * Unsets the busy sending header state.
   *
   * @return true if not cancelled
   */
  private boolean unsetBusySendingHeaders() {
    return (mState.updateAndGet(BUSY_SENDING_HEADER_UNSETTER) & State.CANCELLED) != State.CANCELLED;
  }

  /**
   * Sets the busy sending data state if not already cancelled.
   *
   * @return true if not already cancelled
   */
  private boolean setBusySendingData() {
    return (mState.updateAndGet(BUSY_SENDING_DATA_SETTER) & State.CANCELLED) == 0;
  }

  /**
   * Unsets the sending data busy state.
   *
   * @return true if not cancelled
   */
  private boolean unsetBusySendingData() {
    return (mState.updateAndGet(BUSY_SENDING_DATA_UNSETTER) & State.CANCELLED) != State.CANCELLED;
  }

  /**
   * Sets the busy reading data state if not already cancelled.
   *
   * @return true if not already cancelled
   */
  private boolean setBusyReadingData() {
    return (mState.updateAndGet(BUSY_READING_DATA_SETTER) & State.CANCELLED) == 0;
  }

  /**
   * Unsets the busy reading data state.
   *
   * @return true if not cancelled
   */
  private boolean unsetBusyReadingData() {
    return (mState.updateAndGet(BUSY_READING_DATA_UNSETTER) & State.CANCELLED) != State.CANCELLED;
  }

  private static class BusyStateSetter implements IntUnaryOperator {

    @State private final int cancelBusyState;

    BusyStateSetter(@State int cancelBusyState) { this.cancelBusyState = cancelBusyState; }

    @Override
    public int applyAsInt(@State int originalCancelBusyState) {
      // If by mistake there are concurrent invocations of this method, then the second Thread will
      // get this AssertionError. This condition would constitute a software bug: by contract, for a
      // given method (readData or sendData), invocations can only happen "one at a time" since we
      // have to wait for an EM callback before being allowed to invoke the given method again. For
      // sendHeaders, the rule is even simpler: only one invocation.
      assert (originalCancelBusyState & cancelBusyState) == 0;
      // For this assert to trigger, is means that stream is not finished being initialized. It is
      // a software bug: very likely setStream has not been invoked yet.
      assert (originalCancelBusyState & State.BUSY_STARTING) == 0;
      return (originalCancelBusyState & State.CANCELLED) != 0
          ? originalCancelBusyState
          : originalCancelBusyState | cancelBusyState;
    }
  }

  private static class BusyStateUnsetter implements IntUnaryOperator {

    @State private final int cancelBusyState;

    BusyStateUnsetter(@State int cancelBusyState) { this.cancelBusyState = cancelBusyState; }

    @Override
    public int applyAsInt(@State int originalCancelBusyState) {
      // Triggering this assert means there is a bug in this class, or setStream was called twice.
      assert (originalCancelBusyState & cancelBusyState) != 0;
      return originalCancelBusyState & ~cancelBusyState;
    }
  }
}

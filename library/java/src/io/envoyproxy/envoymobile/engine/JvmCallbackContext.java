package io.envoyproxy.envoymobile.engine;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

class JvmCallbackContext {
  private enum FrameType {
    NONE,
    HEADERS,
    METADATA,
    TRAILERS,
  }

  private final AtomicBoolean closed = new AtomicBoolean(false);
  private final EnvoyHTTPCallbacks callbacks;

  // State-tracking for header accumulation
  private Map<String, List<String>> headerAccumulator = null;
  private FrameType pendingFrameType = FrameType.NONE;
  private boolean pendingEndStream = false;
  private long expectedHeaderLength = 0;
  private long accumulatedHeaderLength = 0;

  public JvmCallbackContext(EnvoyHTTPCallbacks callbacks) { this.callbacks = callbacks; }

  /**
   * Return whether a callback should be allowed to continue with execution. This ensures at most
   * one 'terminal' callback is issued for any given stream.
   *
   * @param close, whether the stream should be closed as part of this determination.
   */
  private boolean dispatchable(boolean close) {
    if (close) {
      // Set closed to true and return true if not previously closed.
      return !closed.getAndSet(true);
    }
    return !closed.get();
  }

  /**
   * Initializes state for accumulating header pairs via passHeaders, ultimately
   * to be dispatched via the callback.
   *
   * @param length,    the total number of headers included in this header block.
   * @param endStream, whether this header block is the final remote frame.
   */
  public void onHeaders(long length, boolean endStream) {
    startAccumulation(FrameType.HEADERS, length, endStream);
  }

  /**
   * Initializes state for accumulating trailer pairs via passHeaders, ultimately
   * to be dispatched via the callback.
   *
   * @param length, the total number of trailers included in this header block.
   */
  public void onTrailers(long length) { startAccumulation(FrameType.TRAILERS, length, true); }

  /**
   * Allows pairs of strings to be passed across the JVM, reducing overall calls
   * (at the expense of some complexity).
   *
   * @param key,        the name of the HTTP header.
   * @param value,      the value of the HTTP header.
   * @param endHeaders, indicates this is the last header pair for this header
   *                    block.
   */
  public void passHeader(byte[] key, byte[] value, boolean endHeaders) {
    String headerKey;
    String headerValue;

    try {
      headerKey = new String(key, "UTF-8");
      headerValue = new String(value, "UTF-8");
    } catch (java.io.UnsupportedEncodingException e) {
      throw new RuntimeException(e);
    }

    List<String> values = headerAccumulator.get(headerKey);
    if (values == null) {
      values = new ArrayList(1);
      headerAccumulator.put(headerKey, values);
    }
    values.add(headerValue);
    accumulatedHeaderLength++;

    if (!endHeaders) {
      return;
    }

    // Received last header, so proceed with dispatch and cleanup
    assert accumulatedHeaderLength == expectedHeaderLength;

    final Map<String, List<String>> headers = headerAccumulator;
    final FrameType frameType = pendingFrameType;
    final boolean endStream = pendingEndStream;

    Runnable runnable = new Runnable() {
      public void run() {
        if (!dispatchable(endStream)) {
          return;
        }

        switch (frameType) {
        case HEADERS:
          callbacks.onHeaders(headers, endStream);
          break;
        case METADATA:
          callbacks.onMetadata(headers);
          break;
        case TRAILERS:
          callbacks.onTrailers(headers);
          break;
        case NONE:
        default:
          assert false : "missing header frame type";
        }
      }
    };

    callbacks.getExecutor().execute(runnable);

    resetHeaderAccumulation();
  }

  /**
   * Dispatches data received from the JNI layer up to the platform.
   *
   * @param data,      chunk of body data from the HTTP response.
   * @param endStream, indicates this is the last remote frame of the stream.
   */
  public void onData(byte[] data, boolean endStream) {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        if (!dispatchable(endStream)) {
          return;
        }
        ByteBuffer dataBuffer = ByteBuffer.wrap(data);
        callbacks.onData(dataBuffer, endStream);
      }
    });
  }

  /**
   * Dispatches error received from the JNI layer up to the platform.
   *
   * @param message,   the error message.
   * @param errorCode, the envoy_error_code_t.
   */
  public void onError(byte[] message, int errorCode) {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        if (!dispatchable(true)) {
          return;
        }
        String errorMessage = new String(message);
        callbacks.onError(errorCode, errorMessage);
      }
    });
  }

  /**
   * Dispatches cancellation notice up to the platform
   */
  public void onCancel() {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        // This call is atomically gated at the call-site and will only happen once.
        callbacks.onCancel();
      }
    });
  }

  /**
   * Cancel the callback context atomically so that no further callbacks occur
   * other than onCancel.
   *
   * @return boolean, whether the callback context was closed or not.
   */
  public boolean cancel() {
    // Atomically close the stream if not already closed.
    boolean closed = dispatchable(true);
    if (closed) {
      // Directly fire callback if closure occurred.
      onCancel();
    }
    return closed;
  }

  private void startAccumulation(FrameType type, long length, boolean endStream) {
    assert headerAccumulator == null;
    assert pendingFrameType == FrameType.NONE;
    assert pendingEndStream == false;
    assert expectedHeaderLength == 0;
    assert accumulatedHeaderLength == 0;

    headerAccumulator = new HashMap((int)length);
    pendingFrameType = type;
    expectedHeaderLength = length;
    pendingEndStream = endStream;
  }

  private void resetHeaderAccumulation() {
    headerAccumulator = null;
    pendingFrameType = FrameType.NONE;
    pendingEndStream = false;
    expectedHeaderLength = 0;
    accumulatedHeaderLength = 0;
  }
}

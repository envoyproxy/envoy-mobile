package io.envoyproxy.envoymobile.engine;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

class JvmCallbackContext {
  private final JvmBridgeUtility bridgeUtility;
  private final EnvoyHTTPCallbacks callbacks;

  public JvmCallbackContext(EnvoyHTTPCallbacks callbacks) {
    bridgeUtility = new JvmBridgeUtility();
    this.callbacks = callbacks;
  }

  /**
   * Initializes state for accumulating header pairs via passHeaders, ultimately
   * to be dispatched via the callback.
   *
   * @param length,    the total number of headers included in this header block.
   * @param endStream, whether this header block is the final remote frame.
   * @return Void,     not used for response callbacks.
   */
  public Void onHeaders(long headerCount, boolean endStream) {
    assert bridgeUtility.validateCount(headerCount);
    final Map headers = bridgeUtility.retrieveHeaders();

    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        callbacks.onHeaders(headers, endStream);
      }
    });

    return null;
  }

  /**
   * Initializes state for accumulating trailer pairs via passHeaders, ultimately
   * to be dispatched via the callback.
   *
   * @param length, the total number of trailers included in this header block.
   * @return Void,  not used for response callbacks.
   */
  public Void onTrailers(long trailerCount, boolean endStream) {
    assert bridgeUtility.validateCount(trailerCount);
    final Map trailers = bridgeUtility.retrieveHeaders();

    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        callbacks.onTrailers(trailers);
      }
    });

    return null;
  }

  /**
   * Dispatches data received from the JNI layer up to the platform.
   *
   * @param data,      chunk of body data from the HTTP response.
   * @param endStream, indicates this is the last remote frame of the stream.
   * @return Void,     not used for response callbacks.
   */
  public Void onData(byte[] data, boolean endStream) {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        ByteBuffer dataBuffer = ByteBuffer.wrap(data);
        callbacks.onData(dataBuffer, endStream);
      }
    });

    return null;
  }

  /**
   * Dispatches error received from the JNI layer up to the platform.
   *
   * @param errorCode,    the error code.
   * @param message,      the error message.
   * @param attemptCount, the number of times an operation was attempted before firing this error.
   * @return Void,        not used for response callbacks.
   */
  public Void onError(int errorCode, byte[] message, int attemptCount) {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        String errorMessage = new String(message);
        callbacks.onError(errorCode, errorMessage, attemptCount);
      }
    });

    return null;
  }

  /**
   * Dispatches cancellation notice up to the platform
   *
   * @return Void, not used for response callbacks.
   */
  public Void onCancel() {
    callbacks.getExecutor().execute(new Runnable() {
      public void run() {
        // This call is atomically gated at the call-site and will only happen once.
        callbacks.onCancel();
      }
    });

    return null;
  }
}

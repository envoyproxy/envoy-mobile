package io.envoyproxy.envoymobile.engine.types;

import java.nio.ByteBuffer;
import java.util.concurrent.Executor;
import java.util.List;
import java.util.Map;

public interface EnvoyHTTPCallbacks {

  Executor getExecutor();

  /**
   * Called when all headers get received on the async HTTP stream.
   *
   * @param headers,   the headers received.
   * @param endStream, whether the response is headers-only.
   */
  void onHeaders(Map<String, List<String>> headers, boolean endStream);

  /**
   * Called when a data frame gets received on the async HTTP stream. This
   * callback can be invoked multiple times if the data gets streamed.
   *
   * @param data,      the buffer of the data received.
   * @param endStream, whether the data is the last data frame.
   */
  void onData(ByteBuffer data, boolean endStream);

  /**
   * Called when all trailers get received on the async HTTP stream. Note that end
   * stream is implied when on_trailers is called.
   *
   * @param trailers, the trailers received.
   */
  void onTrailers(Map<String, List<String>> trailers);

  /**
   * Called when the async HTTP stream has an error.
   *
   * @param errorCode,    the error code.
   * @param message,      the error message.
   * @param attemptCount, the number of times an operation was attempted before firing this error.
   *                      -1 is used in scenarios where it does not make sense to have an attempt
   *                      count for an error. This is different from 0, which intentionally conveys
   *                      that the action was _not_ executed.
   */
  void onError(int errorCode, String message, int attemptCount);

  /**
   * Called when the async HTTP stream is canceled.
   */
  void onCancel();
}

package io.envoyproxy.envoymobile.engine.types;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;

public interface EnvoyObserver {
  /**
   * Called when all headers get received on the async HTTP stream.
   *
   * @param headers,   the headers received.
   * @param endStream, whether the response is headers-only.
   *                   execution.
   */
  void onHeaders(Map<String, List<String>> headers, boolean endStream);

  /**
   * Called when a data frame gets received on the async HTTP stream.
   * This callback can be invoked multiple times if the data gets streamed.
   *
   * @param data,      the buffer of the data received.
   * @param endStream, whether the data is the last data frame.
   *                   execution.
   */
  void onData(ByteBuffer data, boolean endStream);

  /**
   * Called when all metadata get received on the async HTTP stream.
   * Note that end stream is implied when on_metadata is called.
   *
   * @param metadata, the metadata received.
   *                  execution.
   */
  void onMetadata(Map<String, List<String>> metadata);

  /**
   * Called when all trailers get received on the async HTTP stream.
   * Note that end stream is implied when on_trailers is called.
   *
   * @param trailers, the trailers received.
   *                  execution.
   */
  void onTrailers(Map<String, List<String>> trailers);

  /**
   * Called when the async HTTP stream has an error.
   */
  void onError();

  /**
   * Called when the async HTTP stream is canceled.
   */
  void onCancel();
}

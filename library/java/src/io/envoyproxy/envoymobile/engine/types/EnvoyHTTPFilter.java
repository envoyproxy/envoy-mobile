package io.envoyproxy.envoymobile.engine.types;

import java.nio.ByteBuffer;
import java.util.concurrent.Executor;
import java.util.List;
import java.util.Map;

public interface EnvoyHTTPFilter {
  /**
   * Called when all headers get received on the async HTTP stream.
   *
   * @param headers,   the headers received.
   * @param endStream, whether the response is headers-only.
   */
  Object[] onRequestHeaders(Map<String, List<String>> headers, boolean endStream);

  /**
   * Called when a data frame gets received on the async HTTP stream. This
   * callback can be invoked multiple times if the data gets streamed.
   *
   * @param data,      the buffer of the data received.
   * @param endStream, whether the data is the last data frame.
   */
  //public abstract List onRequestData(ByteBuffer data, boolean endStream);

  /**
   * Called when all trailers get received on the async HTTP stream. Note that end
   * stream is implied when on_trailers is called.
   *
   * @param trailers, the trailers received.
   */
  //public abstract void onRequestTrailers(Map<String, List<String>> trailers);

  /**
   * Called when all headers get received on the async HTTP stream.
   *
   * @param headers,   the headers received.
   * @param endStream, whether the response is headers-only.
   */
  Object[] onResponseHeaders(Map<String, List<String>> headers, boolean endStream);

  /**
   * Called when a data frame gets received on the async HTTP stream. This
   * callback can be invoked multiple times if the data gets streamed.
   *
   * @param data,      the buffer of the data received.
   * @param endStream, whether the data is the last data frame.
   */
  //public abstract void onResponseData(ByteBuffer data, boolean endStream);

  /**
   * Called when all trailers get received on the async HTTP stream. Note that end
   * stream is implied when on_trailers is called.
   *
   * @param trailers, the trailers received.
   */
  //public abstract void onResponseTrailers(Map<String, List<String>> trailers);
}

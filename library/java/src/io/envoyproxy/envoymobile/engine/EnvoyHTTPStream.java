package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyData;
import io.envoyproxy.envoymobile.engine.types.EnvoyHeaders;
import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;
import io.envoyproxy.envoymobile.engine.types.EnvoyStream;

public class EnvoyHTTPStream {

  private final EnvoyStream stream;

  EnvoyHTTPStream(EnvoyObserver envoyObserver) {
    this.stream = JniLibrary.startStream(envoyObserver);
  }

  /**
   * Send headers over an open HTTP stream. This method can be invoked once and needs to be called
   * before send_data.
   *
   * @param headers,   the headers to send.
   * @param endStream, supplies whether this is headers only.
   */
  public void sendHeaders(EnvoyHeaders headers, boolean endStream) {
    JniLibrary.sendHeaders(stream, headers, endStream);
  }

  /**
   * Send data over an open HTTP stream. This method can be invoked multiple times.
   *
   * @param data,      the data to send.
   * @param endStream, supplies whether this is the last data in the stream.
   */
  public void sendData(EnvoyData data, boolean endStream) {
    JniLibrary.sendData(stream, data, endStream);
  }

  /**
   * Send metadata over an HTTP stream. This method can be invoked multiple times.
   *
   * @param metadata,  the metadata to send.
   * @param endStream, supplies whether this is the last data in the stream.
   */
  public void sendMetadata(EnvoyHeaders metadata, boolean endStream) {
    JniLibrary.sendMetadata(stream, metadata, endStream);
  }

  /**
   * Send trailers over an open HTTP stream. This method can only be invoked once per stream.
   * Note that this method implicitly ends the stream.
   *
   * @param trailers, the trailers to send.
   */
  public void sendTrailers(EnvoyHeaders trailers) { JniLibrary.sendTrailers(stream, trailers); }

  /**
   * Cancel the stream. This functions as an interrupt, and aborts further callbacks and handling of
   * the stream.
   *
   * @return Success, unless the stream has already been canceled.
   */
  public int cancel() { return JniLibrary.cancel(stream); }
}

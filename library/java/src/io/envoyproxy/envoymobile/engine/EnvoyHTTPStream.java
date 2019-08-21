package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

import java.util.List;
import java.util.Map;

public class EnvoyHTTPStream {

  private final long streamHandle;

  EnvoyHTTPStream(long streamHandle, EnvoyObserver observer) {
    this.streamHandle = streamHandle;
    JniLibrary.startStream(streamHandle, observer);
  }

  /**
   * Send headers over an open HTTP streamHandle. This method can be invoked once and needs to be
   * called before send_data.
   *
   * @param headers,   the headers to send.
   * @param endStream, supplies whether this is headers only.
   */
  public void sendHeaders(Map<String, List<String>> headers, boolean endStream) {
    JniLibrary.sendHeaders(streamHandle, headers, endStream);
  }

  /**
   * Send data over an open HTTP streamHandle. This method can be invoked multiple times.
   *
   * @param data,      the data to send.
   * @param endStream, supplies whether this is the last data in the streamHandle.
   */
  public void sendData(byte[] data, boolean endStream) {
    JniLibrary.sendData(streamHandle, data, endStream);
  }

  /**
   * Send metadata over an HTTP streamHandle. This method can be invoked multiple times.
   *
   * @param metadata, the metadata to send.
   */
  public void sendMetadata(Map<String, List<String>> metadata) {
    JniLibrary.sendMetadata(streamHandle, metadata);
  }

  /**
   * Send trailers over an open HTTP streamHandle. This method can only be invoked once per
   * streamHandle. Note that this method implicitly ends the streamHandle.
   *
   * @param trailers, the trailers to send.
   */
  public void sendTrailers(Map<String, List<String>> trailers) {
    JniLibrary.sendTrailers(streamHandle, trailers);
  }

  /**
   * Cancel the streamHandle. This functions as an interrupt, and aborts further callbacks and
   * handling of the streamHandle.
   *
   * @return Success, unless the streamHandle has already been canceled.
   */
  public int resetStream() { return JniLibrary.resetStream(streamHandle); }
}

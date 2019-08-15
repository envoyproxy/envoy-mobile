package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyData;
import io.envoyproxy.envoymobile.engine.types.EnvoyHeaders;
import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

class JniLibrary {

  protected static native int runEngine(String config, String logLevel);

  protected static native long startStream(EnvoyObserver observer);

  protected static native int sendHeaders(long stream, EnvoyHeaders headers, boolean endStream);

  protected static native int sendData(long stream, EnvoyData data, boolean endStream);

  protected static native int sendMetadata(long stream, EnvoyHeaders metadata);

  protected static native int sendTrailers(long stream, EnvoyHeaders trailers);

  protected static native int locallyCloseStream(long stream);

  protected static native int resetStream(long stream);

  protected static native int cancel(long stream);
}

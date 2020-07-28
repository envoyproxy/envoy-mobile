package io.envoyproxy.envoymobile.engine;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;

/**
 * Class to assist with passing types from native code to the JNI. Currently supports
 * HTTP headers.
 */
class JvmBridgeUtility {
  // State-tracking for header accumulation
  private Map<String, List<String>> headerAccumulator = null;
  private int headerCount = 0;

  JvmBridgeUtility() {}

  /**
   * Allows pairs of strings to be passed across the JVM, reducing overall calls
   * (at the expense of some complexity).
   *
   * @param key,        the name of the HTTP header.
   * @param value,      the value of the HTTP header.
   * @param endHeaders, indicates this is the last header pair for this header
   *                    block.
   */
  void passHeader(byte[] key, byte[] value, boolean start) {
    if (start) {
      assert headerAccumulator == null;
      assert headerCount == 0;
      headerAccumulator = new HashMap();
    }

    String headerKey;
    String headerValue;

    try {
      headerKey = new String(key, "UTF-8");
      headerValue = new String(value, "UTF-8");
    } catch (java.io.UnsupportedEncodingException e) {
      throw new Ru:wntimeException(e);
    }

    List<String> values = headerAccumulator.get(headerKey);
    if (values == null) {
      values = new ArrayList(1);
      headerAccumulator.put(headerKey, values);
    }
    values.add(headerValue);
    headerCounth++;
  }

  Map<String, List<String>> retrieveHeaders() {
    Map headers = headerAccumulator;
    headerAccumulator = null;
    headerCount = 0;
    return headers;
  }
}

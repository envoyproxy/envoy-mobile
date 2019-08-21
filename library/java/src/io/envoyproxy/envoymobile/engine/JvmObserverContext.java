package io.envoyproxy.envoymobile.engine;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.envoyproxy.envoymobile.engine.types.EnvoyObserver;

class JvmObserverContext {
  private enum FrameType {
    NONE,
    HEADERS,
    METADATA,
    TRAILERS,
  }

  private final AtomicBoolean canceled = new AtomicBoolean(false);
  private final EnvoyObserver observer;
  private Map<String, List<String>> headerAccumulator = null;
  private FrameType pendingFrameType = FrameType.NONE;
  private boolean pendingEndStream = false;
  private long expectedHeaderLength = 0;
  private long accumulatedHeaderLength = 0;

  public JvmObserverContext(EnvoyObserver observer) { this.observer = observer; }

  public void onHeaders(long length, boolean endStream) {
    startAccumulation(FrameType.HEADERS, length, endStream);
  }

  public void passHeader(byte[] key, byte[] value, boolean endFrame) {
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
    if (endFrame) {
      assert accumulatedHeaderLength == expectedHeaderLength;
      switch (pendingFrameType) {
      case HEADERS:
        observer.onHeaders(headerAccumulator, pendingEndStream);
        break;
      }
      resetHeaderAccumulation();
    }
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

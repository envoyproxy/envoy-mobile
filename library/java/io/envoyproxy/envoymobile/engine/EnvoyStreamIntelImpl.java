package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyStreamIntel;

class EnvoyStreamIntelImpl implements EnvoyStreamIntel {
  private long streamId;
  private long connectionId;
  private long attemptCount;
  private long receivedByteCount;

  EnvoyStreamIntelImpl(long[] values) {
    streamId = values[0];
    connectionId = values[1];
    attemptCount = values[2];
    receivedByteCount = values[3];
  }

  @Override
  public long getStreamId() {
    return streamId;
  }

  @Override
  public long getConnectionId() {
    return connectionId;
  }

  @Override
  public long getAttemptCount() {
    return attemptCount;
  }

  @Override
  public long getReceivedByteCount() {
    return receivedByteCount;
  }
}

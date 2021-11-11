package io.envoyproxy.envoymobile.engine.types;

/**
 * Exposes internal HTTP stream metrics, context, and other details.
 */
public interface EnvoyStreamMetrics {
  // TODO(alyssawilk) comment.
  public long getRequestStartMs();
  public long getDnsStartMs();
  public long getDnsEndMs();
  public long getConnectStartMs();
  public long getConnectEndMs();
  public long getSslStartMs();
  public long getSslEndMs();
  public long getSendingStartMs();
  public long getSendingEndMs();
  public long getResponseStartMs();
  public long getRequestEndMs();
  public boolean getSocketReused();
  public long getSentByteCount();
  public long getReceivedByteCount();
}

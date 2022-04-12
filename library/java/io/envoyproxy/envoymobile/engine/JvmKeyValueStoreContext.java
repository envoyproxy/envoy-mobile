package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyKeyValueStore;
import java.nio.charset.StandardCharsets;

/**
 * JNI compatibility class to translate calls to EnvoyKeyValueStore implementaitons.
 *
 * Dealing with Java Strings directly in the JNI is awkward due to how Java encodes them.
 */
class JvmKeyValueStoreContext {
  private final EnvoyKeyValueStore keyValueStore;

  public JvmKeyValueStoreContext(EnvoyKeyValueStore keyValueStore) {
    this.keyValueStore = keyValueStore;
  }

  public byte[] read(byte[] key) {
    return keyValueStore.read(new String(key, StandardCharsets.UTF_8))
        .getBytes(StandardCharsets.UTF_8);
  }

  public void remove(byte[] key) { keyValueStore.remove(new String(key, StandardCharsets.UTF_8)); }

  public void save(byte[] key, byte[] value) {
    keyValueStore.save(new String(key, StandardCharsets.UTF_8),
                       new String(value, StandardCharsets.UTF_8));
  }
}

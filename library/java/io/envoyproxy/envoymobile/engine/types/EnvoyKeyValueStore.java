package io.envoyproxy.envoymobile.engine.types;

public interface EnvoyKeyValueStore {
  String read(String key);
  void remove(String key);
  void store(String key, String value);
}

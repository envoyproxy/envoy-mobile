package io.envoyproxy.envoymobile.engine.types;

// A Simple pair of strings
public class EnvoyMobilePair {
    private final String key;
    private final String value;

    public EnvoyMobilePair(String key, String value) {
        this.key = key;
        this.value = value;
    }
    public String getKey() {
        return key;
    }

    public String getValue() {
        return value;
    }
}
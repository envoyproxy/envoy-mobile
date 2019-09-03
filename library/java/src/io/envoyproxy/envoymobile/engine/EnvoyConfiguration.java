package io.envoyproxy.envoymobile.engine;

public class EnvoyConfiguration {

  public final String configYAML;
  public final Integer connectTimeoutSeconds;
  public final Integer dnsRefreshSeconds;
  public final Integer statsFlushSeconds;

  public EnvoyConfiguration(String configYAML) {
    this.configYAML = configYAML;
    this.connectTimeoutSeconds = null;
    this.dnsRefreshSeconds = null;
    this.statsFlushSeconds = null;
  }

  public EnvoyConfiguration(int connectTimeoutSeconds, int dnsRefreshSeconds, int statsFlushSeconds) {
    this.configYAML = null;
    this.connectTimeoutSeconds = connectTimeoutSeconds;
    this.dnsRefreshSeconds = dnsRefreshSeconds;
    this.statsFlushSeconds = statsFlushSeconds;
  }



  public String resolve(String defaultConfigurationYAML) {
    String resolvedConfiguration =
        defaultConfigurationYAML
            .replace("{{ connect_timeout }}", String.format("%ss", connectTimeoutSeconds))
            .replace("{{ dns_refresh_rate }}", String.format("%ss", dnsRefreshSeconds))
            .replace("{{ stats_flush_interval }}", String.format("%ss", statsFlushSeconds));

    if (resolvedConfiguration.contains("{{")) {
      throw new ConfigurationException();
    }
    return resolvedConfiguration;
  }

  static class ConfigurationException extends RuntimeException {
    ConfigurationException() {
      super("Unresolved Template Key");
    }
  }
}

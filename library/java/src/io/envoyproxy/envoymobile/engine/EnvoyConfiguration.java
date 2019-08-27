package io.envoyproxy.envoymobile.engine;

public class EnvoyConfiguration {

  public final String configYAML;
  public final int connectTimeoutSeconds;
  public final int dnsRefreshSeconds;
  public final int statsFlushSeconds;

  public EnvoyConfiguration(String configYAML, int connectTimeoutSeconds, int dnsRefreshSeconds,
                            int statsFlushSeconds) {
    this.configYAML = configYAML;
    this.connectTimeoutSeconds = connectTimeoutSeconds;
    this.dnsRefreshSeconds = dnsRefreshSeconds;
    this.statsFlushSeconds = statsFlushSeconds;
  }

  public String resolve(String defaultConfigurationYAML) {
    String resolvedYAML;
    if (configYAML == null) {
      resolvedYAML = defaultConfigurationYAML;
    } else {
      resolvedYAML = configYAML;
    }
    String resolvedConfiguration =
        resolvedYAML
            .replace("{{ connect_timeout }}", String.format("%ss", connectTimeoutSeconds))
            .replace("{{ dns_refresh_rate }}", String.format("%ss", dnsRefreshSeconds))
            .replace("{{ stats_flush_interval }}", String.format("%ss", statsFlushSeconds));
    if (resolvedConfiguration.contains("{{")) {
      throw new ConfigurationException();
    }
    return resolvedConfiguration;
  }

  static class ConfigurationException extends RuntimeException {
    ConfigurationException() { super("Unresolved Template Key"); }
  }
}

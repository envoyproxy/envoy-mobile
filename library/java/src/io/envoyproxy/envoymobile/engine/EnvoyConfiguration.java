package io.envoyproxy.envoymobile.engine;

public class EnvoyConfiguration {

  public final String configYAML;
  public final Integer connectTimeoutSeconds;
  public final Integer dnsRefreshSeconds;
  public final Integer statsFlushSeconds;

  /**
   * Create an EnvoyConfiguration with a user provided configuration YAML.
   *
   * @param configYAML configuration to be used for starting Envoy.
   */
  public EnvoyConfiguration(String configYAML) {
    this.configYAML = configYAML;
    this.connectTimeoutSeconds = null;
    this.dnsRefreshSeconds = null;
    this.statsFlushSeconds = null;
  }

  /**
   * Create an EnvoyConfiguration with a user provided configuration values.
   *
   * @param connectTimeoutSeconds timeout for new network connections to hosts in the cluster.
   * @param dnsRefreshSeconds rate in seconds to refresh DNS.
   * @param statsFlushSeconds interval at which to flush Envoy stats.
   */
  public EnvoyConfiguration(int connectTimeoutSeconds, int dnsRefreshSeconds,
                            int statsFlushSeconds) {
    this.configYAML = null;
    this.connectTimeoutSeconds = connectTimeoutSeconds;
    this.dnsRefreshSeconds = dnsRefreshSeconds;
    this.statsFlushSeconds = statsFlushSeconds;
  }

  /**
   * Resolves the provided default configuration with the field values:
   * - connectTimeoutSeconds
   * - dnsRefreshSeconds
   * - statsFlushSeconds
   * This default configuration is provided by the native layer.
   *
   * @param defaultConfigurationYAML the default template configuration.
   * @return String, the resolved template.
   * @throws ConfigurationException, when the template provided is not fully resolved.
   */
  String resolveTemplate(String defaultConfigurationYAML) {
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
    ConfigurationException() { super("Unresolved Template Key"); }
  }
}

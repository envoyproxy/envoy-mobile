package io.envoyproxy.envoymobile.engine;

import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import javax.annotation.Nullable;

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory;
import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

/* Typed configuration that may be used for starting Envoy. */
public class EnvoyConfiguration {
  public final String grpcStatsDomain;
  public final Integer statsdPort;
  public final Integer connectTimeoutSeconds;
  public final Integer dnsRefreshSeconds;
  public final Integer dnsFailureRefreshSecondsBase;
  public final Integer dnsFailureRefreshSecondsMax;
  public final String dnsPreresolveHostnames;
  public final List<EnvoyHTTPFilterFactory> httpPlatformFilterFactories;
  public final Integer statsFlushSeconds;
  public final Integer streamIdleTimeoutSeconds;
  public final String appVersion;
  public final String appId;
  public final String virtualClusters;
  public final List<EnvoyNativeFilterConfig> nativeFilterChain;
  public final Map<String, EnvoyStringAccessor> stringAccessors;

  private static final Pattern UNRESOLVED_KEY_PATTERN = Pattern.compile("\\{\\{ (.+) \\}\\}");

  /**
   * Create a new instance of the configuration.
   *
   * @param grpcStatsDomain              the domain to flush stats to.
   * @param connectTimeoutSeconds        timeout for new network connections to hosts in
   *                                     the cluster.
   * @param dnsRefreshSeconds            rate in seconds to refresh DNS.
   * @param dnsFailureRefreshSecondsBase base rate in seconds to refresh DNS on failure.
   * @param dnsFailureRefreshSecondsMax  max rate in seconds to refresh DNS on failure.
   * @param dnsPreresolveHostnames       hostnames to preresolve on Envoy Client construction.
   * @param statsFlushSeconds            interval at which to flush Envoy stats.
   * @param streamIdleTimeoutSeconds     idle timeout for HTTP streams.
   * @param appVersion                   the App Version of the App using this Envoy Client.
   * @param appId                        the App ID of the App using this Envoy Client.
   * @param virtualClusters              the JSON list of virtual cluster configs.
   * @param nativeFilterChain            the configuration for native filters.
   * @param httpPlatformFilterFactories  the configuration for platform filters.
   * @param stringAccessors              platform string accessors to register.
   */
  public EnvoyConfiguration(String grpcStatsDomain, @Nullable Integer statsdPort,
                            int connectTimeoutSeconds, int dnsRefreshSeconds,
                            int dnsFailureRefreshSecondsBase, int dnsFailureRefreshSecondsMax,
                            String dnsPreresolveHostnames, int statsFlushSeconds,
                            int streamIdleTimeoutSeconds, String appVersion, String appId,
                            String virtualClusters, List<EnvoyNativeFilterConfig> nativeFilterChain,
                            List<EnvoyHTTPFilterFactory> httpPlatformFilterFactories,
                            Map<String, EnvoyStringAccessor> stringAccessors) {
    this.grpcStatsDomain = grpcStatsDomain;
    this.statsdPort = statsdPort;
    this.connectTimeoutSeconds = connectTimeoutSeconds;
    this.dnsRefreshSeconds = dnsRefreshSeconds;
    this.dnsFailureRefreshSecondsBase = dnsFailureRefreshSecondsBase;
    this.dnsFailureRefreshSecondsMax = dnsFailureRefreshSecondsMax;
    this.dnsPreresolveHostnames = dnsPreresolveHostnames;
    this.statsFlushSeconds = statsFlushSeconds;
    this.streamIdleTimeoutSeconds = streamIdleTimeoutSeconds;
    this.appVersion = appVersion;
    this.appId = appId;
    this.virtualClusters = virtualClusters;
    this.nativeFilterChain = nativeFilterChain;
    this.httpPlatformFilterFactories = httpPlatformFilterFactories;
    this.stringAccessors = stringAccessors;
  }

  /**
   * Resolves the provided configuration template using properties on this
   * configuration.
   *
   * @param templateYAML the template configuration to resolve.
   * @param platformFilterTemplateYAML helper template to build platform http filters.
   * @param nativeFilterTemplateYAML helper template to build native http filters.
   * @return String, the resolved template.
   * @throws ConfigurationException, when the template provided is not fully
   *                                 resolved.
   */
  String resolveTemplate(final String templateYAML, final String platformFilterTemplateYAML,
                         final String nativeFilterTemplateYAML) {
    final StringBuilder customFiltersBuilder = new StringBuilder();

    for (EnvoyHTTPFilterFactory filterFactory : httpPlatformFilterFactories) {
      String filterConfig = platformFilterTemplateYAML.replace("{{ platform_filter_name }}",
                                                               filterFactory.getFilterName());
      customFiltersBuilder.append(filterConfig);
    }

    for (EnvoyNativeFilterConfig filter : nativeFilterChain) {
      String filterConfig =
          nativeFilterTemplateYAML.replace("{{ native_filter_name }}", filter.name)
              .replace("{{ native_filter_typed_config }}", filter.typedConfig);
      customFiltersBuilder.append(filterConfig);
    }

    String processedTemplate =
        templateYAML.replace("#{custom_filters}", customFiltersBuilder.toString());

    StringBuilder configBuilder = new StringBuilder("!ignore platform_defs:\n");
    configBuilder.append(String.format("- &connect_timeout %ss\n", connectTimeoutSeconds))
        .append(String.format("- &dns_refresh_rate %ss\n", dnsRefreshSeconds))
        .append(String.format("- &dns_fail_base_interval %ss\n", dnsFailureRefreshSecondsBase))
        .append(String.format("- &dns_fail_max_interval %ss\n", dnsFailureRefreshSecondsMax))
        .append(String.format("- &dns_preresolve_hostnames %s\n", dnsPreresolveHostnames))
        .append(String.format("- &stream_idle_timeout %ss\n", streamIdleTimeoutSeconds))
        .append(String.format("- &metadata { device_os: %s, app_version: %s, app_id: %s }\n",
                              "Android", appVersion, appId))
        .append("- &virtual_clusters ")
        .append(virtualClusters)
        .append("\n");

    // TODO(goaway): enable support for both types of sinks, since it's now much easier.
    if (statsdPort != null && grpcStatsDomain != null) {
      throw new ConfigurationException("cannot enable both statsD and gRPC metrics sink");
    } else if (grpcStatsDomain != null) {
      configBuilder.append("- &stats_domain ").append(grpcStatsDomain).append("\n");
      configBuilder.append(String.format("- &stats_flush_interval %ss\n", statsFlushSeconds));
      configBuilder.append("- &stats_sinks [ *base_metrics_service ]\n");
    } else if (statsdPort != null) {
      configBuilder.append("- &statsd_port ").append(statsdPort).append("\n");
      configBuilder.append("- &stats_sinks [ *base_statsd ]\n");
    }

    configBuilder.append(processedTemplate);
    String resolvedConfiguration = configBuilder.toString();

    final Matcher unresolvedKeys = UNRESOLVED_KEY_PATTERN.matcher(resolvedConfiguration);
    if (unresolvedKeys.find()) {
      throw new ConfigurationException(unresolvedKeys.group(1));
    }
    return resolvedConfiguration;
  }

  static class ConfigurationException extends RuntimeException {
    ConfigurationException(String unresolvedKey) {
      super("Unresolved template key: " + unresolvedKey);
    }
  }
}

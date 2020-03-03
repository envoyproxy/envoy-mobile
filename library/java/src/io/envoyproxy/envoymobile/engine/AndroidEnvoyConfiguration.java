package io.envoyproxy.envoymobile.engine;

import android.app.Application;

/* Android-specific configuration that may be used for starting Envoy. */
public class AndroidEnvoyConfiguration extends EnvoyConfiguration {
  public final Application appForLifecycleHandling;

  /**
   * Create a new instance of the configuration.
   *
   * @param statsDomain                  the domain to flush stats to.
   * @param connectTimeoutSeconds        timeout for new network connections to
   *                                     hosts in the cluster.
   * @param dnsRefreshSeconds            rate in seconds to refresh DNS.
   * @param dnsFailureRefreshSecondsBase base rate in seconds to refresh DNS on
   *                                     failure.
   * @param dnsFailureRefreshSecondsMax  max rate in seconds to refresh DNS on
   *                                     failure.
   * @param statsFlushSeconds            interval at which to flush Envoy stats.
   * @param appForLifecycleHandling      the app to use for registering lifecycle
   *                                     handler callbacks.
   */
  public AndroidEnvoyConfiguration(String statsDomain, int connectTimeoutSeconds,
                                   int dnsRefreshSeconds, int dnsFailureRefreshSecondsBase,
                                   int dnsFailureRefreshSecondsMax, int statsFlushSeconds,
                                   Application appForLifecycleHandling) {
    super(statsDomain, connectTimeoutSeconds, dnsRefreshSeconds, dnsFailureRefreshSecondsBase,
          dnsFailureRefreshSecondsMax, statsFlushSeconds);
    this.appForLifecycleHandling = appForLifecycleHandling;
  }
}

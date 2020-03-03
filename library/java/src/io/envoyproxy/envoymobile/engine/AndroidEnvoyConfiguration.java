package io.envoyproxy.envoymobile.engine;

public class AndroidEnvoyConfiguration extends EnvoyConfiguration {
    /**
     * Create an EnvoyConfiguration with a user provided configuration values.
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
     * @param appForLifecycleHandling
     */
    public AndroidEnvoyConfiguration(String statsDomain, int connectTimeoutSeconds, int dnsRefreshSeconds, int dnsFailureRefreshSecondsBase, int dnsFailureRefreshSecondsMax, int statsFlushSeconds, Application appForLifecycleHandling) {
        super(statsDomain, connectTimeoutSeconds, dnsRefreshSeconds, dnsFailureRefreshSecondsBase, dnsFailureRefreshSecondsMax, statsFlushSeconds, appForLifecycleHandling);
}}

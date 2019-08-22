package io.envoyproxy.envoymobile.engine;

public class EnvoyConfigurationImpl implements EnvoyConfiguration {

  /**
   * Provides a default configuration template that may be used for starting Envoy.
   *
   * @return A template that may be used as a starting point for constructing configurations.
   */
  @Override
  public String templateString() {
    // TODO(buildbreaker): templatize this with the JNI Library calls.
    return "{static_resources: {listeners: [{address: {socket_address: {address: 0.0.0.0, port_value: 9001, protocol: TCP}}, filter_chains: [{filters: [{name: envoy.http_connection_manager, typed_config: {'@type': type.googleapis.com/envoy.config.filter.network.http_connection_manager.v2.HttpConnectionManager, stat_prefix: base, route_config: {virtual_hosts: [{name: all, domains: ['*'], routes: [{match: {prefix: /}, route: {cluster: base}}]}]}, http_filters: [{name: envoy.router}]}}]}]}], clusters: [{name: base, connect_timeout: 30s, dns_refresh_rate: 30s, dns_lookup_family: V4_ONLY, lb_policy: ROUND_ROBIN, load_assignment: {cluster_name: base, endpoints: [{lb_endpoints: [{endpoint: {address: {socket_address: {address: s3.amazonaws.com, port_value: 443}}}}]}]}, tls_context: {sni: s3.amazonaws.com}, type: LOGICAL_DNS}]}, stats_flush_interval: 60s, watchdog: {megamiss_timeout: 60s, miss_timeout: 60s}}";
  }
}

package test.kotlin.integration.proxy

import android.content.Context

import io.envoyproxy.envoymobile.AndroidEngineBuilder
import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.Engine

import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
// import org.assertj.core.api.Assertions.assertThat

private val configTemplate =
"""
static_resources:
  listeners:
  - name: base_api_listener
    address:
      socket_address: { protocol: TCP, address: 127.0.0.1, port_value: 10001 }
    api_listener:
      api_listener:
        "@type": "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.EnvoyMobileHttpConnectionManager"
        config:
          stat_prefix: api_hcm
          route_config:
            name: api_router
            virtual_hosts:
            - name: api
              domains: ["*"]
              routes:
              - match: { prefix: "/" }
                direct_response: { status: 400, body: { inline_string: "not found" } }
  - name: listener_proxy
    address:
      socket_address: { address: 127.0.0.1, port_value: 9999 }
    filter_chains:
      - filters:
        - name: envoy.filters.network.http_connection_manager
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
            stat_prefix: remote_hcm
            route_config:
              name: remote_route
              virtual_hosts:
              - name: remote_service
                domains: ["*"]
                routes:
                - match: { prefix: "/" }
                  route: { cluster: cluster_proxy }
              request_headers_to_add:
                - append_action: OVERWRITE_IF_EXISTS_OR_ADD
                  header:
                    key: x-proxy-response
                    value: 'true'
            http_filters:
              - name: envoy.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
  clusters:
  - name: cluster_proxy
    connect_timeout: 30s
    type: LOGICAL_DNS
    cluster_type:
      name: envoy.clusters.dynamic_forward_proxy
      typed_config:
        "@type": type.googleapis.com/envoy.extensions.clusters.dynamic_forward_proxy.v3.ClusterConfig
        dns_cache_config: *dns_cache_config
    dns_lookup_family: ALL
    load_assignment:
      cluster_name: cluster_proxy
      endpoints:
        - lb_endpoints:
            - endpoint:
                address:
                  socket_address:
                    address: api.lyft.com
                    port_value: 80
"""

class Proxy constructor(val context: Context) {
    fun start(timeoutSeconds: Long, port: Int): Engine {      
      val config = String.format(configTemplate, port)
      val proxyBuilder = AndroidEngineBuilder(context, Custom(configTemplate))

      val onEngineRunningLatch = CountDownLatch(1)    
      proxyBuilder.setOnEngineRunning { onEngineRunningLatch.countDown() }
      val engine = proxyBuilder.build()

      onEngineRunningLatch.await(timeoutSeconds, TimeUnit.SECONDS)
      // assertThat(onEngineRunningLatch.count).isEqualTo(0)
    
      return engine
    }
}


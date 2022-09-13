package test.kotlin.integration

import android.content.Context
import android.net.ConnectivityManager
import android.net.ProxyInfo

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.AndroidEngineBuilder
import io.envoyproxy.envoymobile.RequestHeadersBuilder
import io.envoyproxy.envoymobile.RequestMethod
import io.envoyproxy.envoymobile.ResponseHeaders
import io.envoyproxy.envoymobile.StreamIntel
import io.envoyproxy.envoymobile.engine.JniLibrary

import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mock
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner

private val remotePort = (10001..11000).random()
private val config =
"""
static_resources:
  listeners:
  - name: base_api_listener
    address:
      socket_address: { protocol: TCP, address: 127.0.0.1, port_value: 10000 }
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
                route: { cluster: base }
          http_filters:
          - name: envoy.filters.http.network_configuration
            typed_config:
              "@type": type.googleapis.com/envoymobile.extensions.filters.http.network_configuration.NetworkConfiguration
              enable_drain_post_dns_refresh: false
              enable_interface_binding: false
          - name: envoy.filters.http.local_error
            typed_config:
              "@type": type.googleapis.com/envoymobile.extensions.filters.http.local_error.LocalError
          - name: envoy.filters.http.dynamic_forward_proxy
            typed_config:   
              "@type": type.googleapis.com/envoy.extensions.filters.http.dynamic_forward_proxy.v3.FilterConfig
              dns_cache_config: &dns_cache_config
                name: base_dns_cache
                dns_lookup_family: ALL
                host_ttl: 86400s
                dns_min_refresh_rate: 20s
                dns_refresh_rate: 60s
                dns_failure_refresh_rate:
                  base_interval: 2s
                  max_interval: 10s
                dns_query_timeout: 25s
                typed_dns_resolver_config:
                  name: envoy.network.dns_resolver.getaddrinfo
                  typed_config: {"@type":"type.googleapis.com/envoy.extensions.network.dns_resolver.getaddrinfo.v3.GetAddrInfoDnsResolverConfig"}
          - name: envoy.router
            typed_config:
              "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
  - name: listener_0
    address:
      socket_address:
        address: 127.0.0.1
        port_value: $remotePort
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
                  direct_response: { status: 200, body: { inline_string: "not found" } }
                  request_headers_to_remove:
                  - x-forwarded-proto
                  - x-envoy-mobile-cluster
                response_headers_to_add:
                  - append_action: OVERWRITE_IF_EXISTS_OR_ADD
                    header:
                      key: x-proxy-response
                      value: 'true'
            http_filters:
              - name: envoy.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
  clusters:
  - name: base
    connect_timeout: 10s
    lb_policy: CLUSTER_PROVIDED
    transport_socket:
    cluster_type:
      name: envoy.clusters.dynamic_forward_proxy
      typed_config:
        "@type": type.googleapis.com/envoy.extensions.clusters.dynamic_forward_proxy.v3.ClusterConfig
        dns_cache_config: *dns_cache_config
    transport_socket:
      name: envoy.transport_sockets.http_11_proxy
      typed_config:
        "@type": type.googleapis.com/envoy.extensions.transport_sockets.http_11_proxy.v3.Http11ProxyUpstreamTransport
        transport_socket:
          name: envoy.transport_sockets.raw_buffer
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.transport_sockets.raw_buffer.v3.RawBuffer
"""

@RunWith(RobolectricTestRunner::class)
class PerformHTTPRequestUsingProxy {
  init {
    JniLibrary.loadTestLibrary()
  }

  private val onEngineRunningLatch = CountDownLatch(1)
  private val onRespondeHeadersLatch = CountDownLatch(1)

  @Test
  fun `performs an HTTP request through a proxy`() {
    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", remotePort))

    val builder = AndroidEngineBuilder(mockContext, Custom(config))
    val engine = builder
        .enableProxySupport(true)
        .setOnEngineRunning { onEngineRunningLatch.countDown() }
        .build()

    onEngineRunningLatch.await(10, TimeUnit.SECONDS)
    assertThat(onEngineRunningLatch.count).isEqualTo(0)

    val requestHeaders = RequestHeadersBuilder(
      method = RequestMethod.GET,
      scheme = "http",
      authority = "foo.com",
      path = "/"
    )
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        assertThat(status).isEqualTo(200)
        assertThat(responseHeaders.value("x-proxy-response")).isEqualTo(listOf("true"))
        onRespondeHeadersLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    onRespondeHeadersLatch.await(10, TimeUnit.SECONDS)
    engine.terminate()
    assertThat(onRespondeHeadersLatch.count).isEqualTo(0)
  }
}

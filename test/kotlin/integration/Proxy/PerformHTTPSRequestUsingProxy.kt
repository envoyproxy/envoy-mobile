package test.kotlin.integration

import android.content.Context
import android.net.ConnectivityManager
import android.net.ProxyInfo
import androidx.test.core.app.ApplicationProvider

import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.UpstreamHttpProtocol
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
                direct_response: { status: 400, body: { inline_string: "not found" } }
  - name: listener_proxy
    address:
      socket_address:
        address: 127.0.0.1
        port_value: 9998
    filter_chains:
      - filters:
        - name: envoy.filters.network.http_connection_manager
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
            stat_prefix: remote_hcm
            route_config:
              name: remote_route
              virtual_hosts:
              - name: api
                domains: ["*"]
                routes:
                - match: { prefix: "/" }
                  route: { cluster: cluster_proxy }
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
  clusters:
  - name: cluster_proxy
    connect_timeout: 30s
    type: LOGICAL_DNS
    dns_lookup_family: V4_ONLY
    load_assignment:
      cluster_name: cluster_proxy
      endpoints:
        - lb_endpoints:
            - endpoint:
                address:
                  socket_address:
                    address: api.lyft.com
                    port_value: 443
    transport_socket:
      name: envoy.transport_sockets.tls
      typed_config:
        "@type": type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.UpstreamTlsContext
"""

@RunWith(RobolectricTestRunner::class)
class PerformHTTPSRequestUsingProxy {
  init {
    JniLibrary.loadTestLibrary()
  }

  private val onProxyEngineRunningLatch = CountDownLatch(1)
  private val onEngineRunningLatch = CountDownLatch(1)
  private val onRespondeHeadersLatch = CountDownLatch(1)

  @Test
  fun `performs an HTTPs request through a proxy`() {
    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", 9998))

    val proxyBuilder = AndroidEngineBuilder(ApplicationProvider.getApplicationContext(), Custom(config))
    proxyBuilder
      .addLogLevel(LogLevel.TRACE)
      .setOnEngineRunning { onProxyEngineRunningLatch.countDown() }
    val proxyEngine = proxyBuilder.build()

    onProxyEngineRunningLatch.await(10, TimeUnit.SECONDS)

    val builder = AndroidEngineBuilder(mockContext)
    val engine = builder
        .addLogLevel(LogLevel.TRACE)
        .enableProxySupport(true)
        .setOnEngineRunning { onEngineRunningLatch.countDown() }
        .build()

    onEngineRunningLatch.await(10, TimeUnit.SECONDS)
    assertThat(onEngineRunningLatch.count).isEqualTo(0)

    // val proxy = Proxy(ApplicationProvider.getApplicationContext())
    // val proxyEngine = proxy.start(10, 9999)

    val requestHeaders = RequestHeadersBuilder(
      method = RequestMethod.GET,
      scheme = "https",
      authority = "api.lyft.com",
      path = "/ping"
    )
      // .addUpstreamHttpProtocol(UpstreamHttpProtocol.HTTP2)
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        // assertThat(status).isEqualTo()
        // assertThat(responseHeaders.value("x-proxy-response")).isEqualTo("true")
        onRespondeHeadersLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    onRespondeHeadersLatch.await(10, TimeUnit.SECONDS)
    engine.terminate()
    proxyEngine.terminate()
    assertThat(onRespondeHeadersLatch.count).isEqualTo(0)
  }
}

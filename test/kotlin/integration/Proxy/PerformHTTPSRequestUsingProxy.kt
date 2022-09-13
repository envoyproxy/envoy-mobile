package test.kotlin.integration.proxy

import android.content.Context
import android.net.ConnectivityManager
import android.net.ProxyInfo
import androidx.test.core.app.ApplicationProvider

import io.envoyproxy.envoymobile.LogLevel
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
                direct_response: { status: 400, body: { inline_string: "not found" } }
  - name: listener_proxy
    address:
      socket_address:
        address: ::1
        port_value: 9999
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
  - name: cluster_proxy
    connect_timeout: 31s
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
"""

@RunWith(RobolectricTestRunner::class)
class PerformHTTPSRequestUsingProxy {
  init {
    JniLibrary.loadTestLibrary()
  }

  private val onEngineRunningLatch = CountDownLatch(2)
  private val onRespondeHeadersLatch = CountDownLatch(1)

  @Test
  fun `performs an HTTPs request through a proxy`() {
    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", 9999))

    val builder = AndroidEngineBuilder(mockContext)
    val engine = builder
        .addLogLevel(LogLevel.TRACE)
        .enableProxySupport(true)
        .setOnEngineRunning { onEngineRunningLatch.countDown() }
        .build()



    val proxyBuilder = AndroidEngineBuilder(mockContext, Custom(config))

    proxyBuilder
      .addLogLevel(LogLevel.TRACE)
      .setOnEngineRunning { onEngineRunningLatch.countDown() }
    val proxyEngine = proxyBuilder.build()

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
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        assertThat(status).isEqualTo(301)
        assertThat(responseHeaders.value("x-proxy-response")).isEqualTo("true")
        onRespondeHeadersLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    onRespondeHeadersLatch.await(10, TimeUnit.SECONDS)
    engine.terminate()
    // proxyEngine.terminate()
    assertThat(onRespondeHeadersLatch.count).isEqualTo(0)
  }
}

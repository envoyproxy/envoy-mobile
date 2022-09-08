package test.kotlin.integration

import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import android.net.ConnectivityManager
import android.net.ProxyInfo
import org.robolectric.Shadows.shadowOf
import org.mockito.Mock
import org.mockito.Mockito
// import org.mockito.Mockito.mock
// import org.mockito.Mockito.never
// import org.mockito.Mockito.spy
// import org.mockito.Mockito.times
// import org.mockito.Mockito.verify
// import org.mockito.Mockito.when

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.AndroidEngineBuilder
import io.envoyproxy.envoymobile.EnvoyError
import io.envoyproxy.envoymobile.FilterDataStatus
import androidx.test.core.app.ApplicationProvider
import io.envoyproxy.envoymobile.FilterHeadersStatus
import io.envoyproxy.envoymobile.FilterTrailersStatus
import io.envoyproxy.envoymobile.FinalStreamIntel
import io.envoyproxy.envoymobile.RequestHeadersBuilder
import io.envoyproxy.envoymobile.RequestMethod
import io.envoyproxy.envoymobile.ResponseFilter
import io.envoyproxy.envoymobile.ResponseHeaders
import io.envoyproxy.envoymobile.ResponseTrailers
import io.envoyproxy.envoymobile.StreamIntel
import io.envoyproxy.envoymobile.UpstreamHttpProtocol
import io.envoyproxy.envoymobile.engine.JniLibrary
import java.nio.ByteBuffer
import android.content.Context;

import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

private val remotePort = (10001..11000).random()
private val config =
"""
static_resources:
  listeners:
  - name: listener_0
    address:
      socket_address:
        address: 0.0.0.0
        port_value: 10000
    filter_chains:
      - filters:
        - name: envoy.filters.network.tcp_proxy
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.filters.network.tcp_proxy.v3.TcpProxy
            stat_prefix: destination
            cluster: cluster_0
  - name: base_api_listener
    address:
      socket_address: { protocol: TCP, address: 0.0.0.0, port_value: 10000 }
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
          - name: envoy.filters.http.local_error
            typed_config:
              "@type": "type.googleapis.com/envoymobile.extensions.filters.http.local_error.LocalError"
          - name: envoy.filters.http.network_configuration
            typed_config:
              "@type": type.googleapis.com/envoymobile.extensions.filters.http.network_configuration.NetworkConfiguration
              enable_drain_post_dns_refresh: false
              enable_interface_binding: false
          - name: envoy.router
            typed_config:
              "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
  clusters:
  - name: base
    connect_timeout: 10s
    lb_policy: ROUND_ROBIN
    transport_socket:
      name: envoy.transport_sockets.http_11_proxy
      typed_config:
        "@type": type.googleapis.com/envoy.extensions.transport_sockets.http_11_proxy.v3.Http11ProxyUpstreamTransport
        transport_socket:
          name: envoy.transport_sockets.raw_buffer
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.transport_sockets.raw_buffer.v3.RawBuffer
  - name: fake_remote
    connect_timeout: 0.25s
    type: STATIC
    lb_policy: ROUND_ROBIN
    load_assignment:
      cluster_name: fake_remote
      endpoints:
      - lb_endpoints:
        - endpoint:
            address:
              socket_address: { address: 127.0.0.1, port_value: $remotePort }
"""

@RunWith(RobolectricTestRunner::class)
class SetProxyTest {
  init {
    JniLibrary.loadTestLibrary()
  }

  private val startExpectation = CountDownLatch(1)
  private val runExpectation = CountDownLatch(1)

  @Test
  fun `connects through proxy to an HTTP domain`() {
    val context: Context = ApplicationProvider.getApplicationContext()

    val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", remotePort))

    val builder = AndroidEngineBuilder(mockContext, Custom(config))
    builder
        .addLogLevel(LogLevel.TRACE)
        .enableProxySupport(true)
        .setOnEngineRunning { startExpectation.countDown() }

    val engine = builder.build()
    val client = engine.streamClient()

    val requestHeaders = RequestHeadersBuilder(
      method = RequestMethod.GET,
      scheme = "http",
      authority = "example.com",
      path = ""
    )
      .build()

    startExpectation.await(5, TimeUnit.SECONDS)
    assertThat(startExpectation.count).isEqualTo(0)

    client.newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        runExpectation.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    runExpectation.await(5, TimeUnit.SECONDS)

    engine.terminate()

    assertThat(runExpectation.count).isEqualTo(0)
  }
}

package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.mockito.Mockito.mock
import java.util.concurrent.Executors
import java.util.concurrent.*


private const val REQUEST_HANDLER_THREAD_NAME = "hello_envoy_kt"
private const val ENVOY_SERVER_HEADER = "server"
private const val REQUEST_AUTHORITY = "api.lyft.com"
private const val REQUEST_PATH = "/ping"
private const val REQUEST_SCHEME = "https"

class BridgeTest {
  private lateinit var clientBuilder: StreamClientBuilder

  @Test
  fun `first attempt`() {
    val config =
    """
static_resources:
  listeners:
  - name: base_api_listener
    address:
      socket_address:
        protocol: TCP
        address: 0.0.0.0
        port_value: 10000
    api_listener:
      api_listener:
        "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
        stat_prefix: hcm
        route_config:
          name: api_router
          virtual_hosts:
            - name: api
              domains:
                - "*"
              routes:
                - match:
                    prefix: "/"
                  direct_response:
                    status: 418
        http_filters:
          - name: envoy.filters.http.assertion
            typed_config:
              "@type": type.googleapis.com/envoymobile.extensions.filters.http.assertion.Assertion
              match_config:
                http_request_headers_match:
                  headers:
                    - name: ":authority"
                      exact_match: api.lyft.com
          - name: envoy.router
            typed_config:
              "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
    """
    clientBuilder = StreamClientBuilder(Custom(config))

    clientBuilder.addLogLevel(LogLevel.DEBUG)
    val streamClient = clientBuilder.build() as EnvoyClient
    val countDownLatch = CountDownLatch(2)

    val requestHeaders = RequestHeadersBuilder(
      RequestMethod.GET, REQUEST_SCHEME, REQUEST_AUTHORITY, REQUEST_PATH
    )
      .addUpstreamHttpProtocol(UpstreamHttpProtocol.HTTP2)
      .build()
    streamClient
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        val message = "received headers with status $status"
                countDownLatch.countDown()

      }
      .setOnResponseData { data, endStream ->
        countDownLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    println("awaiting!!")
    countDownLatch.await(20, TimeUnit.SECONDS)
    println("done waiting!!")

  }
}

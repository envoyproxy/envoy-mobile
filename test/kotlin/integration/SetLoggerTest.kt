package test.kotlin.integration

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.engine.JniLibrary
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

private const val apiListenerType =
  "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager"
private const val assertionFilterType = "type.googleapis.com/envoymobile.extensions.filters.http.assertion.Assertion"
private const val config =
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
            "@type": $apiListenerType
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
                        status: 200
            http_filters:
              - name: envoy.filters.http.assertion
                typed_config:
                  "@type": $assertionFilterType
                  match_config:
                    http_request_headers_match:
                      headers:
                        - name: ":authority"
                          exact_match: example.com
              - name: envoy.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
    """

class SetLoggerTest {

  init {
    JniLibrary.loadTestLibrary()
  }

  @Test
  fun `set logger`() {
    val countDownLatch = CountDownLatch(1)
    val client = EngineBuilder(Custom(config))
      .addLogLevel(LogLevel.DEBUG)
      .setLogger { msg ->
        if (msg.contains("starting main dispatch loop")) {
          countDownLatch.countDown()
        }
      }
      .setOnEngineRunning {}
      .build()

    countDownLatch.await(30, TimeUnit.SECONDS)
    client.terminate()
    assertThat(countDownLatch.count).isEqualTo(0)
  }

  @Test
  fun `engine should continue to run if no logger is set`() {
    val countDownLatch = CountDownLatch(1)
    val client = EngineBuilder(Custom(config))
      .addLogLevel(LogLevel.DEBUG)
      .setOnEngineRunning {
        countDownLatch.countDown()
      }
      .build()

    countDownLatch.await(30, TimeUnit.SECONDS)
    client.terminate()
    assertThat(countDownLatch.count).isEqualTo(0)
  }
}

package test.kotlin.integration

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.engine.JniLibrary
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

const val apiListenerType =
  "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager"
const val assertionFilterType = "type.googleapis.com/envoymobile.extensions.filters.http.assertion.Assertion"
const val config =
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
    var result: String? = null
    val countDownLatch = CountDownLatch(1)
    EngineBuilder(Custom(config))
      .addLogLevel(LogLevel.TRACE)
      .setLogger { msg ->
        if (msg.contains("starting main dispatch loop")) {
          result = msg
          countDownLatch.countDown()
        }
      }
      .setOnEngineRunning {

      }
      .build()
      .streamClient()
    Thread.sleep(15000)
    countDownLatch.await(15, TimeUnit.SECONDS)
    assertThat(result).contains("starting main dispatch loop")
    assertThat(false).isTrue()
  }
}

package io.envoyproxy.envoymobile.io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.EnvoyClientBuilder
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.mockito.Mockito.mock

private const val TEST_CONFIG = """
mock_template:
- name: mock
  connect_timeout: {{ connect_timeout }}
  dns_refresh_rate: {{ dns_refresh_rate }}
  stats_flush_interval: {{ stats_flush_interval }}
"""

class EnvoyBuilderTest {

  private lateinit var clientBuilder: EnvoyClientBuilder

  private var engine: EnvoyEngine = mock(EnvoyEngine::class.java)

  @Test
  fun `adding custom config builder uses custom config for running Envoy`() {
    clientBuilder = EnvoyClientBuilder()
    clientBuilder.addConfigYAML(TEST_CONFIG)
    clientBuilder.addEngineType { engine }

    clientBuilder.addConfigYAML("mock_template:")
    val envoy = clientBuilder.build()
    assertThat(envoy.configurationYAML).isEqualTo("mock_template:")
  }

  @Test
  fun `adding log level builder uses log level for running Envoy`() {
    clientBuilder = EnvoyClientBuilder()
    clientBuilder.addConfigYAML(TEST_CONFIG)
    clientBuilder.addEngineType { engine }

    clientBuilder.addLogLevel(LogLevel.DEBUG)
    val envoy = clientBuilder.build()
    assertThat(envoy.logLevel).isEqualTo(LogLevel.DEBUG)
  }

  @Test
  fun `specifying connection timeout overrides default`() {
    clientBuilder = EnvoyClientBuilder()
    clientBuilder.addEngineType { engine }

    clientBuilder.addConnectTimeoutSeconds(1234)
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.connectTimeoutSeconds).isEqualTo(1234)
  }

  @Test
  fun `specifying DNS refresh overrides default`() {
    clientBuilder = EnvoyClientBuilder()
    clientBuilder.addEngineType { engine }

    clientBuilder.addDNSRefreshSeconds(1234)
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.dnsRefreshSeconds).isEqualTo(1234)
  }

  @Test
  fun `specifying stats flush overrides default`() {
    clientBuilder = EnvoyClientBuilder()
    clientBuilder.addEngineType { engine }

    clientBuilder.addStatsFlushSeconds(1234)
    clientBuilder.build()
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.statsFlushSeconds).isEqualTo(1234)

  }
}

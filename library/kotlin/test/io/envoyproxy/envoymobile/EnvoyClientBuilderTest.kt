package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.mockito.Mockito.mock

class EnvoyBuilderTest {

  private lateinit var clientBuilder: EnvoyClientBuilder

  private var engine: EnvoyEngine = mock(EnvoyEngine::class.java)

  @Test
  fun `adding log level builder uses log level for running Envoy`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addLogLevel(LogLevel.DEBUG)
    val envoy = clientBuilder.build()
    assertThat(envoy.logLevel).isEqualTo(LogLevel.DEBUG)
  }

  @Test
  fun `specifying stats domain overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addStatsDomain("stats.foo.com")
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.statsDomain).isEqualTo("stats.foo.com")
  }

  @Test
  fun `specifying connection timeout overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addConnectTimeoutSeconds(1234)
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.connectTimeoutSeconds).isEqualTo(1234)
  }

  @Test
  fun `specifying DNS refresh overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addDNSRefreshSeconds(1234)
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.dnsRefreshSeconds).isEqualTo(1234)
  }

  @Test
  fun `specifying DNS failure refresh overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addDNSFailureRefreshSeconds(1234, 5678)
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.dnsFailureRefreshSecondsBase).isEqualTo(1234)
    assertThat(envoy.envoyConfiguration!!.dnsFailureRefreshSecondsMax).isEqualTo(5678)
  }

  @Test
  fun `specifying stats flush overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addStatsFlushSeconds(1234)
    clientBuilder.build()
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.statsFlushSeconds).isEqualTo(1234)
  }

  @Test
  fun `specifying app version overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addAppVersion("version")
    clientBuilder.build()
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.appVersion).isEqualTo("version")
  }

  @Test
  fun `specifying app id overrides default`() {
    clientBuilder = EnvoyClientBuilder(Standard())
    clientBuilder.addEngineType { engine }

    clientBuilder.addAppId("id")
    clientBuilder.build()
    val envoy = clientBuilder.build()
    assertThat(envoy.envoyConfiguration!!.appId).isEqualTo("id")
  }
}

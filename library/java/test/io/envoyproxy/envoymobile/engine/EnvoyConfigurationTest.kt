package io.envoyproxy.envoymobile.engine

//private const val TEST_CONFIG = """
//mock_template:
//- name: mock
//  connect_timeout: {{ connect_timeout }}
//  dns_refresh_rate: {{ dns_refresh_rate }}
//  stats_flush_interval: {{ stats_flush_interval }}
//"""


class EnvoyConfigurationTest {
// TODO: Write these tests well

//  @Test
//  fun `specifying connection timeout overrides default`() {
//    builder = EnvoyBuilder()
//    builder.addConfigYAML(TEST_CONFIG)
//    builder.addEngineType { engine }
//
//    builder.addConnectTimeoutSeconds(1234)
//    val envoy = builder.build()
//
//    assertThat(envoy.envoyConfiguration).contains("connect_timeout: 1234s")
//  }
//
//  @Test
//  fun `specifying DNS refresh overrides default`() {
//    builder = EnvoyBuilder()
//    builder.addConfigYAML(TEST_CONFIG)
//    builder.addEngineType { engine }
//
//    builder.addDNSRefreshSeconds(1234)
//    val envoy = builder.build()
//    assertThat(envoy.envoyConfiguration).contains("dns_refresh_rate: 1234s")
//  }
//
//  @Test
//  fun `specifying stats flush overrides default`() {
//    builder = EnvoyBuilder()
//    builder.addConfigYAML(TEST_CONFIG)
//    builder.addEngineType { engine }
//
//    builder.addStatsFlushSeconds(1234)
//    builder.build()
//    val envoy = builder.build()
//    assertThat(envoy.envoyConfiguration).contains("stats_flush_interval: 1234s")
//
//  }
//
//  @Test(expected = ConfigurationException::class)
//  fun `specifying configs with invalid templates will throw on build`() {
//    builder = EnvoyBuilder()
//    builder.addConfigYAML(TEST_CONFIG)
//    builder.addEngineType { engine }
//
//    builder.addConfigYAML("{{ missing }}")
//    builder.build()
//  }

}
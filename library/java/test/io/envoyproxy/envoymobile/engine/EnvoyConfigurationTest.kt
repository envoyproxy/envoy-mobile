package io.envoyproxy.envoymobile.engine

import org.junit.Test

private const val TEST_CONFIG = """
mock_template:
- name: mock
  connect_timeout: {{ connect_timeout_seconds }}
  dns_refresh_rate: {{ dns_refresh_rate_seconds }}
  stats_flush_interval: {{ stats_flush_interval_seconds }}
"""


class EnvoyConfigurationTest {

  @Test
  fun `resolving with default configuration resolves with values`() {
    val envoyConfiguration = EnvoyConfiguration(123, 234, 345)

    val resolvedTemplate = envoyConfiguration.resolveTemplate(TEST_CONFIG)
    assertThat(resolvedTemplate).contains("connect_timeout: 123s")
    assertThat(resolvedTemplate).contains("dns_refresh_rate: 234s")
    assertThat(resolvedTemplate).contains("stats_flush_interval: 345s")
  }


  @Test(expected = ConfigurationException::class)
  fun `resolve templates with invalid templates will throw on build`() {
    val envoyConfiguration = EnvoyConfiguration(123, 234, 345)

    envoyConfiguration.resolveTemplate("{{ }}")
  }
}

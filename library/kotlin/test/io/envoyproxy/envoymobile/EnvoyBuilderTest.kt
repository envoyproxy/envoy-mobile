package io.envoyproxy.envoymobile.io.envoyproxy.envoymobile

import com.nhaarman.mockitokotlin2.*
import io.envoyproxy.envoymobile.ConfigurationException
import io.envoyproxy.envoymobile.EnvoyBuilder
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.engine.EnvoyConfiguration
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

private const val TEST_CONFIG =
    "mock_template:\n" +
        "- name: mock\n" +
        "  connect_timeout: {{ connect_timeout }}\n" +
        "  dns_refresh_rate: {{ dns_refresh_rate }}\n" +
        "  stats_flush_interval: {{ stats_flush_interval }}"

class EnvoyBuilderTest {

  private lateinit var builder: EnvoyBuilder

  private var envoyConfiguration: EnvoyConfiguration = mock()
  private var engine: EnvoyEngine = mock()

  @Test
  fun `adding custom config builder uses custom config for running Envoy`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.addConfigYAML("mock_template:\n")
    builder.build()
    verify(engine.runWithConfig(eq("mock_template:\n"), any()))
  }

  @Test
  fun `adding log level builder uses log level for running Envoy`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.addLogLevel(LogLevel.DEBUG)
    builder.build()
    verify(engine.runWithConfig(any(), eq(LogLevel.DEBUG.name)))

  }

  @Test
  fun `specifying connection timeout overrides default`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.withDNSRefreshSeconds(1234)
    builder.build()
    val captor = argumentCaptor<String>()
    verify(engine.runWithConfig(captor.capture(), any()))
    assertThat(captor.firstValue).contains("connect_timeout: 1234s")
  }

  @Test
  fun `specifying DNS refresh overrides default`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.withDNSRefreshSeconds(1234)
    builder.build()
    val captor = argumentCaptor<String>()
    verify(engine.runWithConfig(captor.capture(), any()))
    assertThat(captor.firstValue).contains("dns_refresh_rate: 1234s")
  }

  @Test
  fun `specifying stats flush overrides default`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.withStatsFlushSeconds(1234)
    builder.build()
    val captor = argumentCaptor<String>()
    verify(engine.runWithConfig(captor.capture(), any()))
    assertThat(captor.firstValue).contains("stats_flush_interval: 1234s")

  }

  @Test(expected = ConfigurationException::class)
  fun `specifying configs with invalid templates will throw on build`() {
    whenever(envoyConfiguration.templateString()).thenReturn(TEST_CONFIG)
    builder = EnvoyBuilder(envoyConfiguration)
    builder.addEngineType { engine }

    builder.addConfigYAML("{{ missing }}")
    builder.build()
  }
}

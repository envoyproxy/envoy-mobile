package io.envoyproxy.envoymobile.engine

import org.assertj.core.api.Assertions.assertThat
import org.junit.Assert.fail
import org.junit.Test

private const val TEST_CONFIG =
  """
mock_template:
- name: mock
  stats_domain: {{ stats_domain }}
  connect_timeout: {{ connect_timeout_seconds }}s
  dns_refresh_rate: {{ dns_refresh_rate_seconds }}s
  dns_failure_refresh_rate:
    base_interval: {{ dns_failure_refresh_rate_seconds_base }}s
    max_interval: {{ dns_failure_refresh_rate_seconds_max }}s
  platform_filter_chain:
{{ platform_filter_chain }}
  native_filter_chain:
{{ native_filter_chain }}
  stats_flush_interval: {{ stats_flush_interval_seconds }}s
  os: {{ device_os }}
  app_version: {{ app_version }}
  app_id: {{ app_id }}
  virtual_clusters: {{ virtual_clusters }}
"""

private const val TEST_CERTIFICATES =
  """
inline_string: |
  Certificate:
      Signature Algorithm: sha1WithRSAEncryption
          Validity
              Not Before: May 20 08:38:15 2009 GMT

  SHA1 Fingerprint=AE:C5:FB:3F:C8:E1:BF:C4:E5:4F:03:07:5A:9A:E8:00:B7:F7:B6:FA
  -----BEGIN CERTIFICATE-----
  MIIGFDCCA/ygAwIBAgIIU+w77vuySF8wDQYJKoZIhvcNAQEFBQAwUTELMAkGA1UE
  -----END CERTIFICATE-----
"""

private const val PLATFORM_FILTER_CONFIG =
"""
    - platform_filter_name: {{ platform_filter_name }}
"""

private const val NATIVE_FILTER_CONFIG =
"""
    - name: {{ native_filter_name }}
      typed_config: {{ native_filter_typed_config }}
"""

class EnvoyConfigurationTest {

  @Test
  fun `resolving with default configuration resolves with values`() {
    val envoyConfiguration = EnvoyConfiguration(
      "stats.foo.com", 123, 234, 345, 456, 567, "v1.2.3", "com.mydomain.myapp", "[test]",
      listOf<EnvoyNativeFilterConfig>(EnvoyNativeFilterConfig("filter_name", "test_config")),
      emptyList(), emptyMap()
    )

    val resolvedTemplate = envoyConfiguration.resolveTemplate(TEST_CONFIG, PLATFORM_FILTER_CONFIG, NATIVE_FILTER_CONFIG, TEST_CERTIFICATES)
    assertThat(resolvedTemplate).contains("stats_domain: stats.foo.com")
    assertThat(resolvedTemplate).contains("connect_timeout: 123s")
    assertThat(resolvedTemplate).contains("dns_refresh_rate: 234s")
    assertThat(resolvedTemplate).contains("base_interval: 345s")
    assertThat(resolvedTemplate).contains("max_interval: 456s")
    assertThat(resolvedTemplate).contains("stats_flush_interval: 567s")
    assertThat(resolvedTemplate).contains("os: Android")
    assertThat(resolvedTemplate).contains("app_version: v1.2.3")
    assertThat(resolvedTemplate).contains("app_id: com.mydomain.myapp")
    assertThat(resolvedTemplate).contains("virtual_clusters: [test]")
    assertThat(resolvedTemplate).contains("filter_name")
    assertThat(resolvedTemplate).contains("test_config")
  }

  @Test
  fun `resolve templates with invalid templates will throw on build`() {
    val envoyConfiguration = EnvoyConfiguration(
      "stats.foo.com", 123, 234, 345, 456, 567, "v1.2.3", "com.mydomain.myapp", "[test]",
      emptyList(), emptyList(), emptyMap()
    )

    try {
      envoyConfiguration.resolveTemplate("{{ missing }}", "", "", "")
      fail("Unresolved configuration keys should trigger exception.")
    } catch (e: EnvoyConfiguration.ConfigurationException) {
      assertThat(e.message).contains("missing")
    }
  }
}

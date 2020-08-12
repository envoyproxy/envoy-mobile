package io.envoyproxy.envoymobile.engine

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class JvmBridgeUtilityTest {

  @Test
  fun `retrieveHeaders produces a Map with all headers provided via passHeaders`() {
    val utility = JvmBridgeUtility()
    utility.passHeader("test-0".toByteArray(), "value-0".toByteArray(), true)
    utility.passHeader("test-1".toByteArray(), "value-1".toByteArray(), false)
    utility.passHeader("test-1".toByteArray(), "value-2".toByteArray(), false)

    val headers = utility.retrieveHeaders()
    val expectedHeaders = mapOf("test-0" to listOf("value-0"),
                                "test-1" to listOf("value-1", "value-2"))

    assertThat(headers).hasSize(2) // Two keys / header name
                       .usingRecursiveComparison().isEqualTo(expectedHeaders)
  }

  @Test
  fun `validateCount returns a boolean that comparison of the  number of header values in the map`() {
    val utility = JvmBridgeUtility()
    utility.passHeader("test-0".toByteArray(), "value-0".toByteArray(), true)
    utility.passHeader("test-1".toByteArray(), "value-1".toByteArray(), false)
    utility.passHeader("test-1".toByteArray(), "value-2".toByteArray(), false)

    assertThat(utility.validateCount(2)).isFalse()
    assertThat(utility.validateCount(3)).isTrue()
    assertThat(utility.validateCount(4)).isFalse()

    // Resets accumulator
    val headers = utility.retrieveHeaders()

    assertThat(utility.validateCount(3)).isFalse()
    assertThat(utility.validateCount(0)).isTrue()
  }

  @Test(expected = AssertionError::class)
  fun `starting a new header block before a previous one is finished is an error`() {
    val utility = JvmBridgeUtility()

    utility.passHeader("test-0".toByteArray(), "value-0".toByteArray(), true)
    utility.passHeader("test-1".toByteArray(), "value-1".toByteArray(), true)
  }

  @Test(expected = AssertionError::class)
  fun `not starting a new header block is an error`() {
    val utility = JvmBridgeUtility()

    utility.passHeader("test-0".toByteArray(), "value-0".toByteArray(), false)
  }
}

package library.kotlin.test

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class SampleTest {

  @Test
  fun sampleTest() {
    assertThat(true).isFalse()
  }
}
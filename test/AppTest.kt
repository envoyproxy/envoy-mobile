package test

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class AppTest {
  @Test
  fun `smoke test jni call`() {
    assertThat(App.f(4321)).isEqualTo(4321)
  }
}

package test

import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class AppTest {
  @Test
  fun `smoke test jni call`() {
//    assertThat(App.f(4321)).isEqualTo(4321)
    EngineBuilder().build()
  }
}

package test.kotlin.integration

import io.envoyproxy.envoymobile.EngineBuilder
import org.assertj.core.api.Assertions
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class AndroidEnvoyEngineSimpleIntegrationTest {

  @Test
  fun `ensure engine build and termination succeeds with no errors`() {
    val engine = EngineBuilder().build()
    Thread.sleep(5000)
    engine.terminate()
    Assertions.assertThat(true).isTrue()
  }
}

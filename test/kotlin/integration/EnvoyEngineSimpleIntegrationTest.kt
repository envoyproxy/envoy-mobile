package test.kotlin.integration

import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.engine.JniLibrary
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class EnvoyEngineSimpleIntegrationTest {

  init {
    System.out.println("~~~~~~~~~~~~~~")
    System.out.println(System.getProperty("envoy_jni_library_name"))
    System.out.println("~~~~~~~~~~~~~~")
    println(System.getProperty("user.dir"))
    System.out.println("~~~~~~~~~~~~~~")
    JniLibrary.loadTestLibrary()
  }
  @Test
  fun `ensure engine build and termination succeeds with no errors`() {
    val engine = EngineBuilder().build()
    Thread.sleep(5000)
    engine.terminate()
    assertThat(true).isFalse()
  }
}

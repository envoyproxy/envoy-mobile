package test.kotlin

import io.envoyproxy.envoymobile.EngineBuilder
import org.junit.Test

class EnvoyEngineExampleTest {

  @Test
  fun test() {
    val engine = EngineBuilder().build()
    engine.streamClient()
  }
}

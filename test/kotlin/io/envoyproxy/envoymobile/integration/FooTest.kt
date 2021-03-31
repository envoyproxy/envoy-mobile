package test.kotlin.io.envoyproxy.envoymobile.integration

import io.envoyproxy.envoymobile.EngineBuilder
import org.junit.Test

class FooTest {

  @Test
  fun tst() {
    println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    println(System.getProperty("java.library.path"))
    println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

    val engine = EngineBuilder().build()
    engine.streamClient()
  }
}

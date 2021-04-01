package test.kotlin.io.envoyproxy.envoymobile.integration

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class FooTest {

  @Test
  fun tst() {
    println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    println(System.getProperty("user.dir"))
    println(System.getProperty("java.library.path"))
    println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

    assertThat(Main().foo()).isEqualTo(42)
  }
}

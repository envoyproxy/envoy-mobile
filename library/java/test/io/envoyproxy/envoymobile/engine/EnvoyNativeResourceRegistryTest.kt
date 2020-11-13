package io.envoyproxy.envoymobile.engine

import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class EnvoyNativeResourceRegistryTest {


  @Test
  fun `release callbacks are invoked when EnvoyNativeResourceWrappers are flagged as unreachable`() {
    val latch = CountDownLatch(1)
    val testHandle: Long = 77
    run {
      // Wrapper is within scope function and will become unreachable.
      val testResourceWrapper = object : EnvoyNativeResourceWrapper {}
      val testResourceReleaser = object : EnvoyNativeResourceReleaser {
        override fun release(nativeHandle: Long) {
          assertThat(nativeHandle).isEqualTo(testHandle)
          latch.countDown() 
        }
      }
      EnvoyNativeResourceRegistry.globalRegister(testResourceWrapper, testHandle, testResourceReleaser)
    }

    var maxIterations = 10000;
    while (!latch.await(10, TimeUnit.MILLISECONDS)) {
      System.runFinalization()
      System.gc()
      maxIterations -= 1
      assertThat(maxIterations > 0).isTrue()
    }
    //assertThat(latch.await(2000, TimeUnit.MILLISECONDS)).isTrue()
  }

  @Test
  fun `release callbakcs are not invoked when EnvoyNativeResourceWrappers remain reachable`() {
    val latch = CountDownLatch(1)
    val testHandle: Long = 77
    // Wrapper is outside scope function and will remain reachable.
    val testResourceWrapper = object : EnvoyNativeResourceWrapper {}
    run {
      val testResourceReleaser = object : EnvoyNativeResourceReleaser {
        override fun release(nativeHandle: Long) {
          // Should be not called.
          latch.countDown() 
        }
      }
      EnvoyNativeResourceRegistry.globalRegister(testResourceWrapper, testHandle, testResourceReleaser)
    }

    System.runFinalization()
    System.gc()
    assertThat(latch.await(2000, TimeUnit.MILLISECONDS)).isFalse()
  }
}

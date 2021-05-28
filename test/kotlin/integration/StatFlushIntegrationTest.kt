package test.kotlin.integration

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.Element
import io.envoyproxy.envoymobile.engine.JniLibrary
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import test.kotlin.integration.TestStatsdServer

class StatFlushIntegrationTest {

  init {
    JniLibrary.loadTestLibrary()
  }

  @Test
  fun `concurrent flushes`() {
    val countDownLatch = CountDownLatch(1)
    val client = EngineBuilder()
      .addLogLevel(LogLevel.INFO)
      .addStatsFlushSeconds(1)
      .setLogger { msg ->
        if (msg.contains("starting main dispatch loop")) {
          countDownLatch.countDown()
        }
      }
      .setOnEngineRunning {}
      .build()

    assertThat(countDownLatch.await(30, TimeUnit.SECONDS)).isTrue();

    repeat(100) {
        client.flushStats()
    }

    client.terminate()
  }

  @Test
  fun `flush flushes to stats sink`() {
    val countDownLatch = CountDownLatch(1)
    val client = EngineBuilder()
      .addLogLevel(LogLevel.DEBUG)
      .addStatsdPort(5555)
      // Really high flush interval so it won't trigger during test execution.
      .addStatsFlushSeconds(100)
      .setLogger { msg ->
        if (msg.contains("starting main dispatch loop")) {
          countDownLatch.countDown()
        }
        System.out.println(msg)
      }
      .setOnEngineRunning {}
      .build()

    assertThat(countDownLatch.await(30, TimeUnit.SECONDS)).isTrue()

    client.pulseClient().counter(Element("foo"), Element("bar")).increment(1)

    try {
        val statsdServer = TestStatsdServer()
        statsdServer.runAsync(5555)

        Thread.sleep(20000)
        statsdServer.requestNextPacketCapture()
        client.flushStats()
        val packet = statsdServer.awaitPacketCapture()
        assertThat(packet).contains("envoy.pulse.foo.bar:1|c")
    } finally {
        client.terminate()
    }
  }
}

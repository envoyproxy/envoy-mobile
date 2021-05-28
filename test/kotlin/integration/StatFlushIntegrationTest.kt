package test.kotlin.integration

import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.EngineBuilder
import io.envoyproxy.envoymobile.Engine
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.Element
import io.envoyproxy.envoymobile.engine.JniLibrary
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.junit.After
import test.kotlin.integration.TestStatsdServer

class StatFlushIntegrationTest {
  private var engine: Engine? = null

  init {
    JniLibrary.loadTestLibrary()
  }

  @After
  fun teardown() {
      engine?.terminate()
      engine = null
  }

  @Test
  fun `concurrent flushes with histograms`() {
    val countDownLatch = CountDownLatch(1)
    engine = EngineBuilder()
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

    engine!!.pulseClient().distribution(Element("something")).recordValue(100)

    repeat(100) {
        engine!!.flushStats()
    }
  }

  @Test
  fun `flush flushes to stats sink`() {
    val countDownLatch = CountDownLatch(1)
    engine = EngineBuilder()
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

    engine!!.pulseClient().counter(Element("foo"), Element("bar")).increment(1)

    val statsdServer = TestStatsdServer()
    statsdServer.runAsync(5555)

    statsdServer.requestNextPacketCapture()
    engine!!.flushStats()
    val packet = statsdServer.awaitPacketCapture()

    assertThat(packet).contains("envoy.pulse.foo.bar:1|c")
  }
}

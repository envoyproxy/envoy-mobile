package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.After
import org.junit.Before
import org.junit.Ignore
import org.junit.Test
import java.net.URL
import java.nio.ByteBuffer
import java.util.concurrent.*
import java.util.concurrent.atomic.AtomicReference
import java.util.zip.GZIPInputStream

class EchoServerTest {

  private val port = 1234
  private val server = EchoServer(port)
  private val executor = Executors.newSingleThreadExecutor()

  @Before
  fun setup() {
    server.start()
  }

//  @After
//  fun tearDown() {
//    server.shutdown()
//  }

  @Test
  @Ignore
  fun `echo localhost`() {
    val url = URL("http://0.0.0.0:$port/")
    val openConnection = url.openConnection()
    openConnection.doOutput = true
    val requestStream = openConnection.getOutputStream()
    requestStream.write("hello_world".toByteArray(Charsets.UTF_8))

    val responseStream = openConnection.getInputStream()
    val responseBody = responseStream.readBytes()

    assertThat(ungzip(responseBody)).isEqualTo("hello_world")
  }

  @Test
  fun `envoy echo`() {
    val countDownLatch = CountDownLatch(1)
    val result = AtomicReference<String?>(null)
    val requestHeaders = RequestHeadersBuilder(RequestMethod.POST, "http", "0.0.0.0:$port", "/").build()
    val client = StreamClientBuilder().addLogLevel(LogLevel.TRACE).build()
    client.newStreamPrototype()
      .setOnResponseHeaders { headers, endStream ->
        countDownLatch.countDown()
      }
      .setOnResponseData { data, endStream ->
        result.set(data.array().toString(Charsets.UTF_8))
        countDownLatch.countDown()
      }
      .setOnResponseTrailers { trailers ->
        countDownLatch.countDown()
      }
      .setOnError { error ->
        countDownLatch.countDown()
      }
      .setOnCancel {
        countDownLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, false)
      .close(ByteBuffer.wrap("hello_envoy".toByteArray(Charsets.UTF_8)))

    countDownLatch.await(15, TimeUnit.SECONDS)
    assertThat(result.get()).isEqualTo("hello_envoy")
  }

  fun ungzip(content: ByteArray): String =
    GZIPInputStream(content.inputStream()).bufferedReader(Charsets.UTF_8).use { it.readText() }
}

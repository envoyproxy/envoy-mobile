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
    val responseBody = responseStream.readBytes().toString(Charsets.UTF_8)

    assertThat(responseBody).isEqualTo("hello_world")
  }

  @Test
  fun `envoy echo`() {
    val countDownLatch = CountDownLatch(1)
    val client = EnvoyClientBuilder()
        .build()
    val request = RequestBuilder(RequestMethod.POST, "http", "localhost:$port", "/")
        .build()
    val result = AtomicReference<String?>(null)
    val emitter = client.send(
        request,
        ByteBuffer.wrap("hello_envoy".toByteArray(Charsets.UTF_8)),
        null,
        ResponseHandler(executor)
            .onHeaders { headers, statusCode, endStream ->
              println("on headers")
            }
            .onData { buffer, _ ->
              result.set(buffer.array().toString(Charsets.UTF_8))
              countDownLatch.countDown()
            }
            .onTrailers { trailers ->
              println("")
              countDownLatch.countDown()
            }
            .onError { error ->
              println("")
              countDownLatch.countDown()
            }
            .onCanceled {
              println("")
              countDownLatch.countDown()
            }
    )

    countDownLatch.await(5, TimeUnit.SECONDS)
    assertThat(result.get()).isEqualTo("hello_envoy")
  }
}
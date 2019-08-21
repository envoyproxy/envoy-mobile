package library.kotlin.test.io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.Envoy
import io.envoyproxy.envoymobile.RequestBuilder
import io.envoyproxy.envoymobile.RequestMethod
import io.envoyproxy.envoymobile.ResponseHandler
import io.envoyproxy.envoymobile.engine.EnvoyEngine
import io.envoyproxy.envoymobile.engine.EnvoyHTTPStream
import org.junit.Test
import org.mockito.ArgumentMatchers.any
import org.mockito.Mockito.`when`
import org.mockito.Mockito.mock
import org.mockito.Mockito.verify
import java.nio.ByteBuffer

class EnvoyTest {

  private val engine = mock(EnvoyEngine::class.java)
  private val stream = mock(EnvoyHTTPStream::class.java)

  @Test
  fun `starting a stream on envoy sends headers`() {
    `when`(engine.startStream(any())).thenReturn(stream)
    val envoy = Envoy(engine, "")

    val headers = mapOf("key_1" to listOf("value_a"))
    envoy.startStream(
        RequestBuilder(
            method = RequestMethod.POST,
            scheme = "https",
            authority = "api.foo.com",
            path = "foo")
            .setHeaders(headers)
            .build(),
        ResponseHandler())

    verify(stream).sendHeaders(headers, false)
  }

  @Test
  fun `sending request on envoy sends headers`() {
    `when`(engine.startStream(any())).thenReturn(stream)
    val envoy = Envoy(engine, "")

    val headers = mapOf("key_1" to listOf("value_a"))
    envoy.send(
        RequestBuilder(
            method = RequestMethod.POST,
            scheme = "https",
            authority = "api.foo.com",
            path = "foo")
            .setHeaders(headers)
            .build(),
        ByteBuffer.allocate(0),
        ResponseHandler())

    verify(stream).sendHeaders(headers, false)
  }

  @Test
  fun `sending request on envoy passes the body buffer`() {
    `when`(engine.startStream(any())).thenReturn(stream)
    val envoy = Envoy(engine, "")

    val body = ByteBuffer.allocate(0)
    envoy.send(
        RequestBuilder(
            method = RequestMethod.POST,
            scheme = "https",
            authority = "api.foo.com",
            path = "foo")
            .build(),
        body,
        ResponseHandler())

    verify(stream).sendData(body, false)
  }

  @Test
  fun `sending request on envoy without trailers sends empty trailers`() {
    `when`(engine.startStream(any())).thenReturn(stream)
    val envoy = Envoy(engine, "")

    val body = ByteBuffer.allocate(0)
    envoy.send(
        RequestBuilder(
            method = RequestMethod.POST,
            scheme = "https",
            authority = "api.foo.com",
            path = "foo")
            .build(),
        body,
        ResponseHandler())

    verify(stream).sendTrailers(emptyMap())
  }

  @Test
  fun `sending request on envoy sends trailers`() {
    `when`(engine.startStream(any())).thenReturn(stream)
    val envoy = Envoy(engine, "")

    val trailers = mapOf("key_1" to listOf("value_a"))
    envoy.send(
        RequestBuilder(
            method = RequestMethod.POST,
            scheme = "https",
            authority = "api.foo.com",
            path = "foo")
            .build(),
        ByteBuffer.allocate(0),
        trailers,
        ResponseHandler())

    verify(stream).sendTrailers(trailers)
  }
}
package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import java.net.URL

class RequestBuilderTest {

  @Test
  fun `adding new headers should append to the list of header keys`() {

  }

  @Test
  fun `removing a header key removes the entire header list`() {

  }

  @Test
  fun `adding new trailers should append to the list of trailers keys`() {

  }

  @Test
  fun `removing a trailer key removes the entire trailer list`() {

  }

  @Test
  fun `removing headers should clear headers in request`() {
    val request = RequestBuilder(URL("http://0.0.0.0:9001/api.lyft.com/demo.txt"), RequestMethod.GET)
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .removeHeader("header_a")
        .build()

    assertThat(request.headers).doesNotContainKey("header_a")
  }

  @Test
  fun `removing trailers should clear trailers in request`() {
    val request = RequestBuilder(URL("http://0.0.0.0:9001/api.lyft.com/demo.txt"), RequestMethod.GET)
        .addBody("data".toByteArray())
        .addTrailer("trailer_a", "value_a1")
        .removeTrailer("trailer_a")
        .build()

    assertThat(request.trailers).doesNotContainKey("trailer_a")
  }

}
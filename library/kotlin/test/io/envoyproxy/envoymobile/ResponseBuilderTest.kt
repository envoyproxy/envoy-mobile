package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class ResponseBuilderTest {

  @Test
  fun `adding response data should have body present in response`() {
    val body = "data".toByteArray()

    val response = ResponseBuilder()
        .addBody(body)
        .build()
    assertThat(response.body).isEqualTo(body)
  }

  @Test
  fun `not adding response data should have null body in response`() {
    val response = ResponseBuilder()
        .build()

    assertThat(response.body).isNull()
  }

  @Test
  fun `adding new headers should append to the list of header keys`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .build()

    assertThat(response.headers["header_a"]).contains("value_a1")
  }

  @Test
  fun `adding new trailers should append to the list of trailers keys`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .build()

    assertThat(response.trailers["trailer_a"]).contains("value_a1")
  }

  @Test
  fun `removing headers should clear headers in response`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .removeHeaders("header_a")
        .build()

    assertThat(response.headers).doesNotContainKey("header_a")
  }

  @Test
  fun `removing a specific header value should not be in response`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .removeHeader("header_a", "value_a1")
        .build()

    assertThat(response.headers["header_a"]).doesNotContain("value_a1")
  }

  @Test
  fun `removing a specific header value should keep the other header values in response`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .removeHeader("header_a", "value_a1")
        .build()

    assertThat(response.headers["header_a"]).contains("value_a2")
  }

  @Test
  fun `adding a specific header value should keep the other header values in response`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .build()

    assertThat(response.headers["header_a"]).containsExactly("value_a1", "value_a2")
  }

  @Test
  fun `removing all header values should remove header list in response`() {
    val response = ResponseBuilder()
        .addHeader("header_a", "value_a1")
        .removeHeader("header_a", "value_a1")
        .build()

    assertThat(response.headers["header_a"]).isNull()
  }

  @Test
  fun `removing trailers should clear trailers in response`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .removeTrailers("trailer_a")
        .build()

    assertThat(response.trailers).doesNotContainKey("trailer_a")
  }

  @Test
  fun `removing a specific trailer value should not be in response`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .removeTrailer("trailer_a", "value_a1")
        .build()

    assertThat(response.trailers["trailer_a"]).doesNotContain("value_a1")
  }

  @Test
  fun `removing a specific trailer value should keep the other trailer values in response`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .removeTrailer("trailer_a", "value_a1")
        .build()

    assertThat(response.trailers["trailer_a"]).contains("value_a2")
  }

  @Test
  fun `adding a specific trailer value should keep the other trailer values in response`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .build()

    assertThat(response.trailers["trailer_a"]).containsExactly("value_a1", "value_a2")
  }

  @Test
  fun `removing all trailer values should remove trailer list in response`() {
    val response = ResponseBuilder()
        .addTrailer("trailer_a", "value_a1")
        .removeTrailer("trailer_a", "value_a1")
        .build()

    assertThat(response.trailers["trailer_a"]).isNull()
  }
}

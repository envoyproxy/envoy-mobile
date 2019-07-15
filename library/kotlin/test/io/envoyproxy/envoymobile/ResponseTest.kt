package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions
import org.junit.Test

class ResponseTest {

  @Test
  fun `response with the same properties should be equal`() {
    val response1 = ResponseBuilder()
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    val response2 = ResponseBuilder()
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    Assertions.assertThat(response1).isEqualTo(response2)
  }

  @Test
  fun `response converted to a builder should build to the same response`() {
    val response = ResponseBuilder()
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    Assertions.assertThat(response).isEqualTo(response.toBuilder().build())
  }
}
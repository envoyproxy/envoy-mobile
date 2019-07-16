package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class ResponseTest {

  @Test
  fun `response with the same properties should be equal`() {
    val response1 = ResponseBuilder()
        .addStatus(202)
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    val response2 = ResponseBuilder()
        .addStatus(202)
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    assertThat(response1).isEqualTo(response2)
  }

  @Test
  fun `response converted to a builder should build to the same response`() {
    val response = ResponseBuilder()
        .addStatus(204)
        .addBody("data".toByteArray())
        .addHeader("header_a", "value_a1")
        .addHeader("header_a", "value_a2")
        .addHeader("header_b", "value_b1")
        .addTrailer("trailer_a", "value_a1")
        .addTrailer("trailer_a", "value_a2")
        .addTrailer("trailer_b", "value_b1")
        .build()

    assertThat(response).isEqualTo(response.toBuilder().build())
  }
}

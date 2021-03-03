package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class GRPCRequestHeadersBuilderTest {
  @Test
  fun `adds scheme to header`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.value(":scheme")).containsExactly("https")
    assertThat(headers.scheme).isEqualTo("https")
  }

  @Test
  fun `adds authority to header`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.value(":authority")).containsExactly("envoyproxy.io")
    assertThat(headers.authority).isEqualTo("envoyproxy.io")
  }

  @Test
  fun `adds path to header`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.value(":path")).containsExactly("/pb.api.v1.Foo/GetBar")
    assertThat(headers.path).isEqualTo("/pb.api.v1.Foo/GetBar")
  }

  @Test
  fun `adds grpc content type header`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.value("content-type")).containsExactly("application/grpc")
  }

  @Test
  fun `adds h2 upstream header`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.value("x-envoy-mobile-upstream-protocol")).containsExactly("http2")
  }

  @Test
  fun `uses http post`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .build()
    assertThat(headers.method).isEqualTo(RequestMethod.POST)
    assertThat(headers.value(":method")).containsExactly("POST")
  }

  @Test
  fun `adds timeout header when set to value`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .addtimeoutMs(200)
      .build()
    assertThat(headers.value("grpc-timeout")).containsExactly("200m")
  }

  @Test
  fun `removes timeout header when set to null`() {
    val headers = GRPCRequestHeadersBuilder("https", "envoyproxy.io", "/pb.api.v1.Foo/GetBar")
      .addtimeoutMs(200)
      .addtimeoutMs(null)
      .build()
    assertThat(headers.value("grpc-timeout")).isNull()
  }
}

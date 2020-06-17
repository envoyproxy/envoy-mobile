package io.envoyproxy.envoymobile

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test

class HeadersBuilderTest {
  @Test
  fun `adding new header adds to list of header keys`() {
    val headers = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "1")
      .add("x-foo", "2")
      .build()
    assertThat(headers.value("x-foo")).containsExactly("1", "2")
  }

  @Test
  fun `removing specific header key removes all of its values`() {
    val headers = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "1")
      .add("x-foo", "2")
      .remove("x-foo")
      .build()

    assertThat(headers.allHeaders()).doesNotContainKey("x-foo")
  }

  @Test
  fun `removing specific header key does not remove other keys`() {
    val headers = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
      .build()
    assertThat(headers.value("x-foo")).containsExactly("123")
    assertThat(headers.value("x-bar")).containsExactly("abc")
  }

  @Test
  fun `setting header replaces existing headers with matching name`() {
    val headers = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .set("x-foo", mutableListOf("abc"))
      .build()
    assertThat(headers.value("x-foo")).containsExactly("abc")
  }

  @Test
  fun `builders are equal if underlying headers are equal`() {
    val builder1 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
    val builder2 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
    assertThat(builder1 == builder2).isTrue
  }

  @Test
  fun `headers are equal if underlying headers are equal`() {
    val headers1 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
      .build()
    val headers2 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
      .build()
    assertThat(headers1 == headers2).isTrue
  }

  @Test
  fun `builder pointers are not equal when instances are different`() {
    val builder1 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
    val builder2 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
    assertThat(builder1 === builder2).isFalse
  }

  @Test
  fun `headers pointers are not equal when instances are different`() {
    val headers1 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
      .build()
    val headers2 = RequestHeadersBuilder(mutableMapOf())
      .add("x-foo", "123")
      .add("x-bar", "abc")
      .build()
    assertThat(headers1 === headers2).isFalse
  }
}

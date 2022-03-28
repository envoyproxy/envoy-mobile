package io.envoyproxy.envoymobile

import com.google.flatbuffers.FlatBufferBuilder
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.mockito.Mockito.mock
import Test.SomeType;

class FlatBuffersTest {

  @Test
  fun `flatbuffers construction`() {
    // This test doesn't really test any functionality, just demonstrates that we are able to utilize flatbuffers pending
    // real usage within the code base.
    val s  = SomeType()
    val f = FlatBufferBuilder()
  }
}

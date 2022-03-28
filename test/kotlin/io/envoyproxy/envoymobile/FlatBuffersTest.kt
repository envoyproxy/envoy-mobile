package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import com.google.flatbuffers.FlatBufferBuilder
import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.mockito.Mockito.mock

class FlatBuffersTest {

  @Test
  fun `flatbuffers construction`() {
// This test doesn't really test any functionality, just demonstrates that we are able to utilize flatbuffers pending
// real usage within the code base.
    engineBuilder = EngineBuilder(Standard())
    engineBuilder.addEngineType { envoyEngine }
    engineBuilder.addNativeFilter("name", "config")

    val engine = engineBuilder.build() as EngineImpl
    assertThat(engine.envoyConfiguration!!.nativeFilterChain.size).isEqualTo(1)
  }
}

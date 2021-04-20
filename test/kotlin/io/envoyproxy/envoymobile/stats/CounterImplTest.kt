package io.envoyproxy.envoymobile

import io.envoyproxy.envoymobile.engine.EnvoyEngine
import org.assertj.core.api.Assertions.assertThat
import org.junit.Before
import org.junit.Test
import org.mockito.ArgumentCaptor
import org.mockito.Captor
import org.mockito.Mockito.mock
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations

class CounterImplTest {

  private var envoyEngine: EnvoyEngine = mock(EnvoyEngine::class.java)

  @Captor
  private lateinit var elementsCaptor: ArgumentCaptor<String>

  @Captor
  private lateinit var tagsCaptor: ArgumentCaptor<MutableMap<String, String>>

  @Before
  fun setup() {
    MockitoAnnotations.initMocks(this)
  }

  @Test
  fun `attach returns a new counter with tags and elements`() {
    val pulseClient = PulseClientImpl(envoyEngine)
    val counterOriginal = pulseClient.counter(
      Element("test"), Element("stat"),
      tags = listOf(Tag("testKey1", "testValue1"), Tag("testKey2", "testValue2"))
    )

    counterOriginal.increment(1)

    val counterNew = counterOriginal.attach(
      listOf(Tag("testKey3", "testValue3"), Tag("testKey4", "testValue4"))
    )

    counterNew.increment(5)

    val countCaptor = ArgumentCaptor.forClass(Int::class.java)

    verify(envoyEngine, times(2)).recordCounterInc(
      elementsCaptor.capture(), tagsCaptor.capture(), countCaptor.capture()
    )

    // Original counter increments with its own elements series and tags
    assertThat(elementsCaptor.getAllValues().get(0)).isEqualTo("test.stat")
    assertThat(countCaptor.getAllValues().get(0)).isEqualTo(1)
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey1")).isEqualTo("testValue1")
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey2")).isEqualTo("testValue2")

    // The new counter increments with its own tags and the same series as the original counter
    assertThat(elementsCaptor.getAllValues().get(1)).isEqualTo("test.stat")
    assertThat(countCaptor.getAllValues().get(1)).isEqualTo(5)
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey3")).isEqualTo("testValue3")
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey4")).isEqualTo("testValue4")
  }
}

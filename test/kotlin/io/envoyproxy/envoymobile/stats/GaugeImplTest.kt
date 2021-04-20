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

class GaugeImplTest {

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
  fun `attach returns a new gauge with tags and elements`() {
    val pulseClient = PulseClientImpl(envoyEngine)
    val gaugeOriginal = pulseClient.gauge(
      Element("test"), Element("stat"),
      tags = listOf(Tag("testKey1", "testValue1"), Tag("testKey2", "testValue2"))
    )

    gaugeOriginal.set(1)

    val gaugeNew = gaugeOriginal.attach(
      listOf(Tag("testKey3", "testValue3"), Tag("testKey4", "testValue4"))
    )

    gaugeNew.set(5)

    val amountCaptor = ArgumentCaptor.forClass(Int::class.java)

    verify(envoyEngine, times(2)).recordGaugeSet(
      elementsCaptor.capture(), tagsCaptor.capture(), amountCaptor.capture()
    )

    // Original gauge sets with its own elements series and tags
    assertThat(elementsCaptor.getAllValues().get(0)).isEqualTo("test.stat")
    assertThat(amountCaptor.getAllValues().get(0)).isEqualTo(1)
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey1")).isEqualTo("testValue1")
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey2")).isEqualTo("testValue2")

    // The new gauge sets with its own tags and the same series as the original gauge
    assertThat(elementsCaptor.getAllValues().get(1)).isEqualTo("test.stat")
    assertThat(amountCaptor.getAllValues().get(1)).isEqualTo(5)
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey3")).isEqualTo("testValue3")
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey4")).isEqualTo("testValue4")
  }
}

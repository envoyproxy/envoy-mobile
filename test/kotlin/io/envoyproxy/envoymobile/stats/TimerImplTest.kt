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

class TimerImplTest {

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
  fun `attach returns a new timer with tags and elements`() {
    val pulseClient = PulseClientImpl(envoyEngine)
    val timerOriginal = pulseClient.timer(
      Element("test"), Element("stat"),
      tags = listOf(Tag("testKey1", "testValue1"), Tag("testKey2", "testValue2"))
    )

    timerOriginal.completeWithDuration(1)

    val timerNew = timerOriginal.attach(
      listOf(Tag("testKey3", "testValue3"), Tag("testKey4", "testValue4"))
    )

    timerNew.completeWithDuration(5)

    val durationCaptor = ArgumentCaptor.forClass(Int::class.java)

    verify(envoyEngine, times(2)).recordHistogramDuration(
      elementsCaptor.capture(), tagsCaptor.capture(), durationCaptor.capture()
    )

    // Original timer sets with its own elements series and tags
    assertThat(elementsCaptor.getAllValues().get(0)).isEqualTo("test.stat")
    assertThat(durationCaptor.getAllValues().get(0)).isEqualTo(1)
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey1")).isEqualTo("testValue1")
    assertThat(tagsCaptor.getAllValues().get(0).get("testKey2")).isEqualTo("testValue2")

    // The new timer sets with its own tags and the same series as the original timer
    assertThat(elementsCaptor.getAllValues().get(1)).isEqualTo("test.stat")
    assertThat(durationCaptor.getAllValues().get(1)).isEqualTo(5)
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey3")).isEqualTo("testValue3")
    assertThat(tagsCaptor.getAllValues().get(1).get("testKey4")).isEqualTo("testValue4")
  }
}

package test.kotlin.integration.proxying

import android.content.Context
import android.net.ConnectivityManager
import android.net.ProxyInfo
import androidx.test.core.app.ApplicationProvider

import io.envoyproxy.envoymobile.AndroidEngineBuilder
import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.Engine
import io.envoyproxy.envoymobile.engine.JniLibrary
import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.RequestHeadersBuilder
import io.envoyproxy.envoymobile.RequestMethod
import io.envoyproxy.envoymobile.ResponseHeaders
import io.envoyproxy.envoymobile.StreamIntel

import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

import org.assertj.core.api.Assertions.assertThat
import org.junit.After
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mock
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class PerformHTTPRequestUsingProxy {
  private lateinit var proxyEngine: Engine
  private lateinit var engine: Engine

  init {
    JniLibrary.loadTestLibrary()
  }

  @Test
  fun `performs an HTTP request through a proxy`() {
    val port = (10001..11000).random()

    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", port))

    val onEngineRunningLatch = CountDownLatch(2)
    val onRespondeHeadersLatch = CountDownLatch(1)

    val proxyEngineBuilder = Proxy(ApplicationProvider.getApplicationContext(), port)
      .http()
    proxyEngine = proxyEngineBuilder
      .addLogLevel(LogLevel.DEBUG)
      .setOnEngineRunning { onEngineRunningLatch.countDown() }
      .build()

    val builder = AndroidEngineBuilder(mockContext)
    engine = builder
      .addLogLevel(LogLevel.DEBUG)
      .enableProxySupport(true)
      .setOnEngineRunning { onEngineRunningLatch.countDown() }
      .build()


    onEngineRunningLatch.await(10, TimeUnit.SECONDS)
    assertThat(onEngineRunningLatch.count).isEqualTo(0)

    val requestHeaders = RequestHeadersBuilder(
      method = RequestMethod.GET,
      scheme = "http",
      authority = "api.lyft.com",
      path = "/ping"
    )
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        assertThat(status).isEqualTo(301)
        assertThat(responseHeaders.value("x-proxy-response")).isEqualTo(listOf("true"))
        onRespondeHeadersLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    onRespondeHeadersLatch.await(15, TimeUnit.SECONDS)
    assertThat(onRespondeHeadersLatch.count).isEqualTo(0)
  }

  @After
  public fun tearDown() {
    engine.terminate()
    proxyEngine.terminate()
  }
}

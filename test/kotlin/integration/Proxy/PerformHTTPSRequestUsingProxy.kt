package test.kotlin.integration.proxy

import android.content.Context
import android.net.ConnectivityManager
import android.net.ProxyInfo
import androidx.test.core.app.ApplicationProvider

import io.envoyproxy.envoymobile.LogLevel
import io.envoyproxy.envoymobile.Custom
import io.envoyproxy.envoymobile.UpstreamHttpProtocol
import io.envoyproxy.envoymobile.AndroidEngineBuilder
import io.envoyproxy.envoymobile.RequestHeadersBuilder
import io.envoyproxy.envoymobile.RequestMethod
import io.envoyproxy.envoymobile.ResponseHeaders
import io.envoyproxy.envoymobile.StreamIntel
import io.envoyproxy.envoymobile.engine.JniLibrary

import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

import org.assertj.core.api.Assertions.assertThat
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mock
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class PerformHTTPSRequestUsingProxy {
  init {
    JniLibrary.loadTestLibrary()
  }

  @Test
  fun `performs an HTTPs request through a proxy`() {
    val mockContext = Mockito.mock(Context::class.java)
    Mockito.`when`(mockContext.getApplicationContext()).thenReturn(mockContext)
    val mockConnectivityManager = Mockito.mock(ConnectivityManager::class.java)
    Mockito.`when`(mockContext.getSystemService(Mockito.anyString())).thenReturn(mockConnectivityManager)
    Mockito.`when`(mockConnectivityManager.getDefaultProxy()).thenReturn(ProxyInfo.buildDirectProxy("127.0.0.1", 9998))

    val onEngineRunningLatch = CountDownLatch(2)
    val onRespondeHeadersLatch = CountDownLatch(1)

    val builder = AndroidEngineBuilder(mockContext)
    val engine = builder
      .addLogLevel(LogLevel.TRACE)
      .enableProxySupport(true)
      .setOnEngineRunning { onEngineRunningLatch.countDown() }
      .build()

    val proxyEngineBuilder = Proxy(ApplicationProvider.getApplicationContext(), 9999).https()
    val proxyEngine = proxyEngineBuilder
      .addLogLevel(LogLevel.TRACE)
      .setOnEngineRunning { onEngineRunningLatch.countDown() }
      .build()

    onEngineRunningLatch.await(10, TimeUnit.SECONDS)
    assertThat(onEngineRunningLatch.count).isEqualTo(0)

    val requestHeaders = RequestHeadersBuilder(
      method = RequestMethod.GET,
      scheme = "https",
      authority = "api.lyft.com",
      path = "/ping"
    )
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _, _ ->
        val status = responseHeaders.httpStatus ?: 0L
        assertThat(status).isEqualTo(200)
        assertThat(responseHeaders.value("x-proxy-response")).isEqualTo(listOf("true"))
        onRespondeHeadersLatch.countDown()
      }
      .start(Executors.newSingleThreadExecutor())
      .sendHeaders(requestHeaders, true)

    onRespondeHeadersLatch.await(10, TimeUnit.SECONDS)
    assertThat(onRespondeHeadersLatch.count).isEqualTo(0)
    engine.terminate()
    proxyEngine.terminate()
  }
}

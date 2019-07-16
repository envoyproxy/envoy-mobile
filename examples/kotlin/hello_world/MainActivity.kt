package io.envoyproxy.envoymobile.helloenvoykotlin

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import android.support.v7.widget.DividerItemDecoration
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.util.Log
import io.envoyproxy.envoymobile.*
import io.envoyproxy.envoymobile.shared.Failure
import io.envoyproxy.envoymobile.shared.Response
import io.envoyproxy.envoymobile.shared.ResponseRecyclerViewAdapter
import io.envoyproxy.envoymobile.shared.Success
import java.io.IOException
import java.io.InputStream
import java.net.URL
import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executor
import java.util.concurrent.TimeUnit


private const val REQUEST_HANDLER_THREAD_NAME = "hello_envoy_kt"
private const val ENDPOINT = "http://0.0.0.0:9001/api.lyft.com/static/demo/hello_world.txt"
private const val ENVOY_SERVER_HEADER = "server"

class MainActivity : Activity() {
  private lateinit var recyclerView: RecyclerView
  private val thread = HandlerThread(REQUEST_HANDLER_THREAD_NAME)
  private lateinit var envoy: Envoy
  private lateinit var client: Client

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.activity_main)

    Envoy.load(baseContext)

    // Create Envoy instance with config.
    envoy = Envoy(baseContext, loadEnvoyConfig(baseContext, R.raw.config))
    client = HttpUrlConnectionClient()

    recyclerView = findViewById(R.id.recycler_view) as RecyclerView
    recyclerView.layoutManager = LinearLayoutManager(this)

    val adapter = ResponseRecyclerViewAdapter()
    recyclerView.adapter = adapter
    val dividerItemDecoration = DividerItemDecoration(recyclerView.context, DividerItemDecoration.VERTICAL)
    recyclerView.addItemDecoration(dividerItemDecoration)
    thread.start()
    val handler = Handler(thread.looper)

    // Run a request loop until the application exits.
    handler.postDelayed(object : Runnable {
      override fun run() {
        try {
          val response = makeRequest()
          recyclerView.post { adapter.add(response) }
        } catch (e: IOException) {
          Log.d("MainActivity", "exception making request.", e)
        }

        // Make a call again
        handler.postDelayed(this, TimeUnit.SECONDS.toMillis(1))
      }
    }, TimeUnit.SECONDS.toMillis(1))
  }

  override fun onDestroy() {
    super.onDestroy()
    thread.quit()
  }

  private fun makeRequest(): Response {

    return try {
      val url = URL(ENDPOINT)

      val latch = CountDownLatch(1)
      var response: io.envoyproxy.envoymobile.Response? = null
      client.request(RequestBuilder(url, RequestMethod.GET)
          .build(), UiThreadExecutor()) { envoyResponse ->
        response = envoyResponse
        latch.countDown()
      }
      latch.await()

      val status = response!!.status
      if (status == 200) {
        val serverHeaderField = response!!.headers[ENVOY_SERVER_HEADER]
        val body = response!!.body.toString()
        Success(body, serverHeaderField?.joinToString(separator = ", ") ?: "")
      } else {
        Failure("failed with status: $status")
      }
    } catch (e: IOException) {
      Failure(e.message ?: "failed with exception")
    }
  }

  private fun deserialize(inputStream: InputStream): String {
    return inputStream.bufferedReader().use { reader -> reader.readText() }
  }

  private fun loadEnvoyConfig(context: Context, configResourceId: Int): String {
    val inputStream = context.getResources().openRawResource(configResourceId)
    return inputStream.bufferedReader().use { reader -> reader.readText() }
  }
}

internal class UiThreadExecutor : Executor {
  private val mHandler = Handler(Looper.getMainLooper())

  override fun execute(command: Runnable) {
    mHandler.post(command)
  }
}
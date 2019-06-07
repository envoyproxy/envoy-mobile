package io.envoyproxy.envoymobile.helloenvoykotlin

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.support.v7.widget.DividerItemDecoration
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.util.Log

import java.io.BufferedReader
import java.io.IOException
import java.io.InputStream
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.TimeUnit

import io.envoyproxy.envoymobile.Envoy

private const val ENDPOINT = "http://0.0.0.0:9001/api.lyft.com/static/demo/hello_world.txt"

class MainActivity : Activity() {
    private lateinit var recyclerView: RecyclerView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val envoy = Envoy()
        envoy.load()
        envoy.run(baseContext, loadEnvoyConfig(baseContext, R.raw.config))

        recyclerView = findViewById(R.id.recycler_view) as RecyclerView
        recyclerView.layoutManager = LinearLayoutManager(this)

        val adapter = ResponseRecyclerViewAdapter()
        recyclerView.adapter = adapter
        val dividerItemDecoration = DividerItemDecoration(recyclerView.context, DividerItemDecoration.VERTICAL)
        recyclerView.addItemDecoration(dividerItemDecoration)
        val thread = HandlerThread("hello_envoy_kt")
        thread.start()
        val handler = Handler(thread.looper)

        handler.postDelayed(object : Runnable {
            override fun run() {
                try {
                    val response = makeRequest()
                    recyclerView.post({ adapter.add(response) })
                } catch (e: IOException) {
                    Log.d("MainActivity", "exception making request.", e)
                }

                // Make a call again
                handler.postDelayed(this, TimeUnit.SECONDS.toMillis(10))
            }
        }, TimeUnit.SECONDS.toMillis(1))
    }

    private fun makeRequest(): Response {
        val url = URL(ENDPOINT)
        val connection = url.openConnection() as HttpURLConnection
        val status = connection.responseCode
        if (status != 200) {
            Log.d("MainActivity", "non 200 status: $status")
        }

        val serverHeaderField = connection.headerFields["server"]
        val inputStream = connection.inputStream
        val body = deserialize(inputStream)
        inputStream.close()
        return Response(body, serverHeaderField?.joinToString(separator = ",") ?: "")
    }

    private fun deserialize(inputStream: InputStream): String {
        return inputStream.bufferedReader().use(BufferedReader::readText)
    }

    private fun loadEnvoyConfig(context: Context, configResourceId: Int): String {
        val inputStream = context.getResources().openRawResource(configResourceId)
        return inputStream.bufferedReader().use { it.readText() }
    }
}


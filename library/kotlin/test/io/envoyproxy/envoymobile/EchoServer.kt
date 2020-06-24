package io.envoyproxy.envoymobile

import com.sun.net.httpserver.HttpExchange
import com.sun.net.httpserver.HttpHandler
import com.sun.net.httpserver.HttpServer
import java.io.ByteArrayOutputStream
import java.net.InetSocketAddress
import java.util.zip.GZIPOutputStream


class EchoServer(
    port: Int
) : HttpHandler {
  private val server: HttpServer = HttpServer.create(InetSocketAddress(port), 0)

  init {
    server.createContext("/", this)
    server.executor = null // creates a default executor
  }

  fun start() {
    server.start()
  }

  fun shutdown() {
    server.stop(0)
  }


  override fun handle(httpExchange: HttpExchange?) {
    val exchange = httpExchange!!
    val requestInputStream = exchange.requestBody
    exchange.responseHeaders["content-encoding"] = "gzip"
    val responseOutputStream = exchange.responseBody
    val responseBody =  gzip(requestInputStream.readBytes().toString(Charsets.UTF_8))
    exchange.sendResponseHeaders(200, responseBody.size.toLong())
    responseOutputStream.write(responseBody)
    responseOutputStream.close()
    requestInputStream.close()
  }
}

fun gzip(content: String): ByteArray {
  val bos = ByteArrayOutputStream()
  GZIPOutputStream(bos).bufferedWriter(Charsets.UTF_8).use { it.write(content) }
  return bos.toByteArray()
}

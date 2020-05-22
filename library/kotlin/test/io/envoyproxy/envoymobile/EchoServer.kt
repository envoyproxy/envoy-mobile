package io.envoyproxy.envoymobile

import com.sun.net.httpserver.HttpExchange
import com.sun.net.httpserver.HttpHandler
import com.sun.net.httpserver.HttpServer
import java.net.InetSocketAddress


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
    val requestBody = requestInputStream.readBytes()

    val responseOutputStream = exchange.responseBody
    exchange.sendResponseHeaders(200, requestBody.size.toLong())
    responseOutputStream.write(requestBody)
    responseOutputStream.close()
    requestInputStream.close()
  }
}
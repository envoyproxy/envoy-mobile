package io.envoyproxy.envoymobile

import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.Executor
import java.util.concurrent.Executors

private const val ENVOY_HOST = "http://0.0.0.0:9001/"

class HttpUrlConnectionClient : Client {
  private val outboundExecutor: Executor = Executors.newFixedThreadPool(10)

  override fun request(request: Request, inboundExecutor: Executor, callback: (Response) -> Unit) {
    outboundExecutor.execute {
      val path = request.url.path
      val resolvedUrl = URL("$ENVOY_HOST$path")


      val urlConnection = resolvedUrl.openConnection() as HttpURLConnection
      urlConnection.requestMethod = method(request.method)

      for (headerEntry in request.headers) {
        for (value in headerEntry.value) {
          urlConnection.setRequestProperty(headerEntry.key, value)
        }
      }
      val responseCode = urlConnection.responseCode
      val responseBuilder = ResponseBuilder()
          .addStatus(responseCode)
          .addBody(urlConnection.responseMessage.toByteArray())

      for (headerEntry in urlConnection.headerFields) {
        if (headerEntry.key == null) {
          // For some reason we get a null value here
          continue
        }
        for (value in headerEntry.value) {
          responseBuilder.addHeader(headerEntry.key, value)
        }
      }

      inboundExecutor.execute {
        callback(responseBuilder.build())
      }
    }
  }

  private fun method(method: RequestMethod): String {
    return when (method) {
      RequestMethod.DELETE -> "DELETE"
      RequestMethod.GET -> "GET"
      RequestMethod.HEAD -> "HEAD"
      RequestMethod.OPTIONS -> "OPTIONS"
      RequestMethod.PATCH -> "PATCH"
      RequestMethod.POST -> "POST"
      RequestMethod.PUT -> "PUT"
      RequestMethod.TRACE -> "TRACE"
    }
  }
}


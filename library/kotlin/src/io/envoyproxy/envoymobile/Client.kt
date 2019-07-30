package io.envoyproxy.envoymobile

import android.content.Context

interface Client {
  /**
   * For starting a stream.
   *
   * @param request the request for opening a stream.
   * @param responseHandler the callback for receiving stream events.
   * @return the emitter for streaming data outward.
   */
  fun startStream(request: Request, responseHandler: ResponseHandler): StreamEmitter
}

abstract class EnvoyClient : Client {
  abstract fun request(request: Request)
}

class CoreClient() : EnvoyClient() {
  override fun startStream(request: Request, responseHandler: ResponseHandler): StreamEmitter {
    TODO("not implemented")
  }

  override fun request(request: Request) {
    TODO("not implemented")
  }
}

class AndroidClient(
  val coreClient: CoreClient,
  val context: Context

) : EnvoyClient() {
  override fun startStream(request: Request, responseHandler: ResponseHandler): StreamEmitter {
    TODO("not implemented")
  }

  override fun request(request: Request) {
    TODO("not implemented")
  }

}

class AndroidEngine(

)

class Engine(

)
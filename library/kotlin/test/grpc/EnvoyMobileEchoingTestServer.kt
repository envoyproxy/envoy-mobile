package io.envoyproxy.envoymobile.grpc

import io.envoyproxy.envoymobile.EnvoyMobileTestGrpc
import io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass
import io.grpc.stub.StreamObserver


class EnvoyMobileEchoingTestServer : EnvoyMobileTestGrpc.EnvoyMobileTestImplBase() {
  override fun stream(responseObserver: StreamObserver<EnvoyMobileTestOuterClass.Response>?): StreamObserver<EnvoyMobileTestOuterClass.Request> {
    return object : StreamObserver<EnvoyMobileTestOuterClass.Request> {
      override fun onNext(p0: EnvoyMobileTestOuterClass.Request?) {
        responseObserver!!.onNext(EnvoyMobileTestOuterClass.Response.newBuilder().setStr(p0!!.str).build())
      }

      override fun onError(p0: Throwable?) {
        responseObserver!!.onError(p0)
      }

      override fun onCompleted() {
        responseObserver!!.onCompleted()
      }
    }
  }

  override fun unary(request: EnvoyMobileTestOuterClass.Request?, responseObserver: StreamObserver<EnvoyMobileTestOuterClass.Response>?) {
    responseObserver!!.onNext(EnvoyMobileTestOuterClass.Response.newBuilder().setStr(request!!.str).build())
    responseObserver.onCompleted()
  }
}
package io.envoyproxy.envoymobile.grpc

import io.envoyproxy.envoymobile.EnvoyMobileTestGrpc
import io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass
import io.grpc.ManagedChannelBuilder
import io.grpc.Server
import io.grpc.ServerBuilder
import org.assertj.core.api.Assertions.assertThat
import org.junit.After
import org.junit.Test

class GrpcTest {

  private val server = ServerBuilder.forPort(1234)
      .addService(EnvoyMobileEchoingTestServer())
      .build()
      .start()

  @After
  fun teardown() {
    server.shutdown()
  }

  @Test
  fun `sanity grpc test`() {
    val channel = ManagedChannelBuilder.forAddress("localhost", 1234)
        .usePlaintext()
        .build()

    val response = EnvoyMobileTestGrpc.newBlockingStub(channel).unary(EnvoyMobileTestOuterClass.Request.newBuilder().setStr("hello_world").build())

    assertThat(response.str).isEqualTo("hello_world")
  }
}

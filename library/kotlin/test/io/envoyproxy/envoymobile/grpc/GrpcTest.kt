package io.envoyproxy.envoymobile.grpc

import io.envoyproxy.envoymobile.Domain
import io.envoyproxy.envoymobile.EnvoyClientBuilder
import io.envoyproxy.envoymobile.GRPCClient
import io.envoyproxy.envoymobile.GRPCRequestBuilder
import io.envoyproxy.envoymobile.GRPCResponseHandler
import io.envoyproxy.envoymobile.LogLevel
import io.grpc.ManagedChannelBuilder
import io.grpc.ServerBuilder
import org.assertj.core.api.Assertions.assertThat
import org.junit.After
import org.junit.Before
import org.junit.Test
import protos.test.EchoServiceGrpc
import protos.test.EchoServiceTest
import java.nio.ByteBuffer
import java.util.concurrent.CountDownLatch
import java.util.concurrent.Executors

class GrpcTest {

  private val server = ServerBuilder.forPort(443)
      .addService(EnvoyMobileEchoingTestServer())
      .executor(Executors.newSingleThreadExecutor())
      .build()

  private val envoyGrpc = GRPCClient(EnvoyClientBuilder(Domain("0.0.0.0")).addLogLevel(LogLevel.DEBUG).build())

  @Before
  fun setup() {
    server.start()
  }

  @After
  fun teardown() {
    server.shutdown()
  }

  //  @Test
  fun `sanity grpc test`() {
    val channel = ManagedChannelBuilder.forAddress("0.0.0.0", 443)
        .usePlaintext()
        .build()

    val response = EchoServiceGrpc.newBlockingStub(channel).unary(EchoServiceTest.EchoServiceRequest.newBuilder().setStr("hello_world").build())

    assertThat(response.str).isEqualTo("hello_world")
  }

  @Test
  fun `envoy grpc test`() {
    val countDownLatch = CountDownLatch(1)
    val stream = envoyGrpc.send(
        GRPCRequestBuilder("/protos.test.EchoService/unary", "0.0.0.0:443", false).build(),
        GRPCResponseHandler(Executors.newSingleThreadExecutor())
            .onHeaders { headers, statusCode, endStream ->
              println("hello")
            }
            .onMessage { byteBuffer ->
              println(byteBuffer)
            }
            .onError {
              println("error")
            }
            .onTrailers {
              println("trailers")
            })
    val request = EchoServiceTest.EchoServiceRequest.newBuilder().setStr("envoy_hello_world").build()
    stream.sendMessage(ByteBuffer.wrap(request.toByteArray()))
    countDownLatch.await()
  }
}


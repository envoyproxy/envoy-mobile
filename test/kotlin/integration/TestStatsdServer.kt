package test.kotlin.integration

import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference
import java.io.IOException
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.SocketTimeoutException

class TestStatsdServer {
  private val shutdownLatch: CountDownLatch = CountDownLatch(1)
  private val failureException: AtomicReference<Exception> = AtomicReference()
  private val latestPacket: AtomicReference<String> = AtomicReference()
  private val nextPacketLatch: AtomicReference<CountDownLatch> = AtomicReference()

  private var thread: Thread? = null

  @Throws(IOException::class)
  fun runAsync(port: Int) {
    val socket = DatagramSocket(port)
    socket.setSoTimeout(1000) // 1 second

    thread = Thread(fun() {
      val buffer = ByteArray(256)
      while (shutdownLatch.getCount() != 0L) {
        val packet = DatagramPacket(buffer, buffer.size)
        try {
          socket.receive(packet)
        } catch (e: SocketTimeoutException) {
          // continue to next loop
          continue
        } catch (e: Exception) {
          failureException.set(e)
          return
        }

        // TODO(snowp): Parse (or use a parser) so we can extract out individual metric names
        // better.
        val received = String(packet.getData(), packet.getOffset(), packet.getLength())
        System.out.println("recevied ${received}")
        val latch = nextPacketLatch.get()
        if (latch != null && latch.getCount() == 1L) {
          latestPacket.set(received)
          latch.countDown()
        }
      }
    });
    thread!!.start();
  }

  fun requestNextPacketCapture() {
    // The idea here is to let the server thread know that it should record the next packet by
    // resetting the latch to 1. Once a packet arrives, we set latestPacket and notify this
    // thread by closing the latch. This works well assuming there are only two interacting threads,
    // the server thread and another awaiting packets.
    nextPacketLatch.set(CountDownLatch(1))
  }

  @Throws(InterruptedException::class)
  fun awaitPacketCapture(): String {
    if (!nextPacketLatch.get().await(20, TimeUnit.SECONDS)) {
      throw RuntimeException("timed out")
    }

    val maybeException = failureException.get()
    if (maybeException != null) {
      throw RuntimeException(maybeException)
    }

    return latestPacket.get()
  }

  @Throws(InterruptedException::class)
  fun shutdown() {
    shutdownLatch.countDown()
    thread?.join()
  }
}

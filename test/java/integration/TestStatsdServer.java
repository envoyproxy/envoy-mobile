package test.kotlin.integration;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketTimeoutException;


public class TestStatsdServer {
    private final CountDownLatch shutdown = new CountDownLatch(1);
    private final AtomicReference<Exception> failure = new AtomicReference<>();
    private final AtomicReference<String> latestPacket = new AtomicReference<>();
    private final AtomicReference<CountDownLatch> nextPacketLatch = new AtomicReference<>();

    private Thread thread;

    void runAsync(int port) throws IOException {
        DatagramSocket socket = new DatagramSocket(port);
        socket.setSoTimeout(1000); // 1 second

        thread = new Thread(() -> {
            byte[] buffer = new byte[256];
            while (shutdown.getCount() != 0) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                try {
                    socket.receive(packet);
                } catch (SocketTimeoutException e) {
                    // continue to next loop
                    continue;
                } catch (Exception e) {
                    failure.set(e);
                    return;
                }

                // TODO(snowp): Parse (or use a parser) so we can extract out individual metric names better.
                String received = new String(packet.getData(), packet.getOffset(), packet.getLength());
                CountDownLatch latch = nextPacketLatch.get();
                if (latch != null && latch.getCount() == 1) {
                    latestPacket.set(received);
                    latch.countDown();
                }
            }
        });
        thread.start();
    }

    String awaitNextPacket() throws InterruptedException {
        // The idea here is to let the server thread know that it should record the next packet by
        // resetting the latch to 1. Once a packet arrives, we set latestPacket and notify this
        // thread by closing the latch. This works well assuming there are only two interacting threads,
        // the server thread and another awaiting packets.
        nextPacketLatch.set(new CountDownLatch(1));

        if (!nextPacketLatch.get().await(20, TimeUnit.SECONDS)) {
            throw new RuntimeException("timed out");
        }
            
        Exception maybeException = failure.get();
        if (maybeException != null) {
            throw new RuntimeException(maybeException);
        }

        return latestPacket.get();
    }

    void shutdown() throws InterruptedException {
        shutdown.countDown();
        thread.join();
    }
}

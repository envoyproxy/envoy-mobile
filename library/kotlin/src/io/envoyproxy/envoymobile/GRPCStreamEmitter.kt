package io.envoyproxy.envoymobile

import java.nio.ByteBuffer


class GRPCStreamEmitter(
    private val emitter: StreamEmitter
) {
  fun sendMessage(message: ByteBuffer) {
    // https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md#requests
    // Length-Prefixed-Message = Compressed-Flag | Message-Length | Message
    // Compressed-Flag = 0 / 1, encoded as 1 byte unsigned integer
    // Message-Length = length of Message, encoded as 4 byte unsigned integer (big endian)
    // Message = binary representation of protobuf message
    val byteBuffer = ByteBuffer.allocate(5)

    // Compression flag (1 byte) - 0, not compressed
    byteBuffer.put(0)

    // Message length
    val messageLength = message.remaining()
    byteBuffer.putInt(messageLength)

    emitter.sendData(byteBuffer)
    emitter.sendData(message)
  }

  fun close() {
    emitter.close(ByteBuffer.wrap(ByteArray(0)))
  }
}

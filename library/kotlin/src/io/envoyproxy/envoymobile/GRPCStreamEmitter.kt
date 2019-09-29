package io.envoyproxy.envoymobile

import java.io.ByteArrayOutputStream
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
    val prefixData = ByteArrayOutputStream()

    // Compression flag (1 byte) - 0, not compressed
    val compressionBit = ByteArray(1)
    compressionBit[0] = 0
    prefixData.write(0)

    // Message length
    val messageLength = message.remaining()
    prefixData.write(messageLength)

  }

  fun close() {
    emitter.close()
  }
}

import Envoy
import Foundation

/// Emitter that allows for sending additional data over gRPC.
@objcMembers
public final class GRPCStreamEmitter: NSObject {
  private let underlyingEmitter: StreamEmitter

  // MARK: - Internal

  /// Initialize a new emitter.
  ///
  /// - parameter emitter: The underlying stream emitter to use for sending data.
  init(emitter: StreamEmitter) {
    self.underlyingEmitter = emitter
  }

  // MARK: - Public

  /// Send a protobuf message's binary data over the gRPC stream.
  ///
  /// - parameter messageData: Binary data of a protobuf message to send.
  public func sendMessage(_ messageData: Data) {
    // https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md#requests
    // Length-Prefixed-Message = Compressed-Flag | Message-Length | Message
    // Compressed-Flag = 0 / 1, encoded as 1 byte unsigned integer
    // Message-Length = length of Message, encoded as 4 byte unsigned integer (big endian)
    // Message = binary representation of protobuf message
    var lengthPrefixedData = Data(capacity: 5 + messageData.count)

    // Compression flag (1 byte) - 0, not compressed
    lengthPrefixedData.append(0)

    // Message length (4 bytes)
    var length = UInt32(messageData.count).bigEndian
    lengthPrefixedData.append(UnsafeBufferPointer(start: &length, count: 1))

    // Message data
    lengthPrefixedData.append(contentsOf: messageData)
    self.underlyingEmitter.sendData(lengthPrefixedData)
  }

  /// Close this connection.
  public func close() {
    self.underlyingEmitter.close()
  }
}

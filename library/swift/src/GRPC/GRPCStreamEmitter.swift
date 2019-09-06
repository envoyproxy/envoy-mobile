import EnvoyInterfaces
import Foundation

/// <#function description#>
@objcMembers
public final class GRPCStreamEmitter: NSObject {
    private let emitter: StreamEmitter

    // MARK: - Internal

    /// <#function description#>
    ///
    /// - parameter emitter: <#emitter description#>
    init(emitter: StreamEmitter) {
        self.emitter = emitter
    }

    // MARK: - Public

    /// <#function description#>
    ///
    /// - parameter messageData: <#messageData description#>
    public func sendMessage(_ messageData: Data) {
        // https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md#requests
        // Length-Prefixed-Message → Compressed-Flag Message-Length Message
        // Compressed-Flag → 0 / 1 # encoded as 1 byte unsigned integer
        // Message-Length → {length of Message} # encoded as 4 byte unsigned integer (big endian)
        // Message → *{binary octet}
        var lengthPrefixedData = Data(capacity: 5 + messageData.count)

        // Compression flag (1 byte) - 0, not compressed
        lengthPrefixedData.append(0)

        // Message length (4 bytes)
        var length = UInt32(messageData.count).bigEndian
        lengthPrefixedData.append(UnsafeBufferPointer(start: &length, count: 1))

        // Message data
        lengthPrefixedData.append(contentsOf: messageData)
        self.emitter.sendData(lengthPrefixedData)
    }

    /// <#function description#>
    public func close() {
        self.emitter.close()
    }
}

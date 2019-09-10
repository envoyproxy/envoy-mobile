import Envoy
import Foundation

/// Handler for responses sent over gRPC.
@objcMembers
public final class GRPCResponseHandler: NSObject {
  /// Represents the state of a response stream's body data.
  enum State {
    /// Awaiting a gRPC compression flag.
    case expectingCompressionFlag
    /// Awaiting the length specification of the next message.
    case expectingMessageLength
    /// Awaiting a message with the specified length.
    case expectingMessage(length: UInt32)
  }

  /// Underlying response handler which should be called with response data.
  let underlyingHandler: ResponseHandler

  /// Initialize a new instance of the handler.
  ///
  /// - parameter queue: Dispatch queue upon which callbacks will be called.
  public init(queue: DispatchQueue = .main) {
    self.underlyingHandler = ResponseHandler(queue: queue)
  }

  /// Specify a callback for when response headers are received by the stream.
  /// If `endStream` is `true`, the stream is complete.
  ///
  /// - parameter closure: Closure which will receive the headers, gRPC code,
  ///                      and flag indicating if the stream is headers-only.
  @discardableResult
  public func onHeaders(_ closure:
    @escaping (_ headers: [String: [String]], _ grpcStatus: Int, _ endStream: Bool) -> Void)
    -> GRPCResponseHandler
  {
    self.underlyingHandler.onHeaders { headers, _, endStream in
      let grpcStatus = GRPCResponseHandler.grpcStatus(fromHeaders: headers)
      closure(headers, grpcStatus, endStream)
    }

    return self
  }

  /// Specify a callback for when a new message has been received by the stream.
  ///
  /// - parameter closure: Closure which will receive messages on the stream,
  ///                      and flag indicating if the stream is complete.
  @discardableResult
  public func onMessage(_ closure:
    @escaping (_ message: Data, _ endStream: Bool) -> Void)
    -> GRPCResponseHandler
  {
    var buffer = Data()
    var state = State.expectingCompressionFlag
    self.underlyingHandler.onData { chunk, endStream in
      buffer.append(chunk)
      GRPCResponseHandler.processBuffer(&buffer, state: &state) { message, isProcessing in
        closure(message, endStream && !isProcessing)
      }
    }

    return self
  }

  /// Specify a callback for when trailers are received by the stream.
  /// If the closure is called, the stream is complete.
  ///
  /// - parameter closure: Closure which will receive the trailers.
  @discardableResult
  public func onTrailers(_ closure:
    @escaping (_ trailers: [String: [String]]) -> Void)
    -> GRPCResponseHandler
  {
    self.underlyingHandler.onTrailers(closure)
    return self
  }

  /// Specify a callback for when an internal Envoy exception occurs with the stream.
  /// If the closure is called, the stream is complete.
  ///
  /// - parameter closure: Closure which will be called when an error occurs.
  @discardableResult
  public func onError(_ closure:
    @escaping (_ error: EnvoyError) -> Void)
    -> GRPCResponseHandler
  {
    self.underlyingHandler.onError(closure)
    return self
  }

  // MARK: - Helpers

  /// Parses out the gRPC status from the provided HTTP headers.
  ///
  /// - parameter headers: The headers from which to obtain the gRPC status.
  ///
  /// - returns: The HTTP status code from the headers, or 0 if none is set.
  static func grpcStatus(fromHeaders headers: [String: [String]]) -> Int {
    return headers["grpc-status"]?
      .compactMap(Int.init)
      .first ?? 0
  }

  /// Recursively processes a buffer of data, buffering it into messages based on state.
  /// When a message has been fully buffered, `onMessage` will be called with the message
  /// and a flag indicating if the buffer is still being processed.
  ///
  /// - parameter buffer:    The buffer of data from which to determine state and messages.
  /// - parameter state:     The current state of the buffering.
  /// - parameter onMessage: Closure to call when a new message is available.
  static func processBuffer(_ buffer: inout Data, state: inout State,
                            onMessage: (_ message: Data, _ isProcessing: Bool) -> Void)
  {
    if buffer.isEmpty {
      return
    }

    switch state {
    case .expectingCompressionFlag:
      guard let compressionFlag: UInt8 = buffer.nextInteger() else {
        return
      }

      guard compressionFlag == 0 else {
        assertionFailure("gRPC decompression is not supported")
        buffer.removeAll()
        state = .expectingCompressionFlag
        return
      }

      buffer.removeFirst()
      state = .expectingMessageLength

    case .expectingMessageLength:
      guard let messageLength: UInt32 = buffer.nextInteger() else {
        return
      }

      buffer.removeFirst(MemoryLayout<UInt32>.size)
      state = .expectingMessage(length: messageLength)

    case .expectingMessage(let length):
      let length = Int(length)
      let message = buffer.withUnsafeBytes { Data(bytes: $0.baseAddress!, count: length) }
      buffer.removeFirst(length)
      onMessage(message, !buffer.isEmpty)
      state = .expectingCompressionFlag
    }

    self.processBuffer(&buffer, state: &state, onMessage: onMessage)
  }
}

private extension Data {
  func nextInteger<T: FixedWidthInteger>() -> T? {
    let size = MemoryLayout<T>.size
    guard self.count >= size else {
      return nil
    }

    var value: T = 0
    _ = Swift.withUnsafeMutableBytes(of: &value) { valuePointer in
      self.copyBytes(to: valuePointer, count: size)
    }

    return value.bigEndian
  }
}

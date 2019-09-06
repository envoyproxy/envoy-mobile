import EnvoyInterfaces
import Foundation

/// <#function description#>
@objcMembers
public final class GRPCResponseHandler: NSObject {
    private var onHeaders: ((_ headers: [String: [String]], _ grpcStatus: Int?) -> Void)?
    private var onMessage: ((_ data: Data) -> Void)?
    private var onComplete: ((_ trailers: [String: [String]]) -> Void)?
    private var state = State.expectingCompressionFlag
    private var buffer: Data!

    /// Underlying response handler which should be called with response data.
    let underlyingHandler = ResponseHandler()

    private enum State {
        case expectingCompressionFlag
        case expectingMessageLength
        case expectingMessage(length: UInt32)
    }

    /// <#function description#>
    ///
    /// - parameter closure: <#closure description#>
    ///
    /// - returns: <#return value description#>
    @discardableResult
    public func onHeaders(_ closure:
        @escaping (_ headers: [String: [String]], _ grpcStatus: Int?) -> Void)
        -> GRPCResponseHandler
    {
        self.onHeaders = closure
        return self
    }

    /// <#function description#>
    ///
    /// - parameter closure: <#closure description#>
    ///
    /// - returns: <#return value description#>
    @discardableResult
    public func onMessage(_ closure:
        @escaping (_ data: Data) -> Void)
        -> GRPCResponseHandler
    {
        self.onMessage = closure
        return self
    }

    /// <#function description#>
    ///
    /// - parameter closure: <#closure description#>
    ///
    /// - returns: <#return value description#>
    @discardableResult
    public func onComplete(_ closure:
        @escaping (_ trailers: [String: [String]]) -> Void)
        -> GRPCResponseHandler
    {
        self.onComplete = closure
        return self
    }

    // MARK: - Internal

    override init() {
        super.init()
        self.underlyingHandler
            .onHeaders { [weak self] headers, _, endStream in
                self?.handleHeaders(headers)
                if endStream {
                    self?.handleCompletion(trailers: [:])
                }
            }
            .onData { [weak self] data, endStream in
                self?.handleData(data)
                if endStream {
                    self?.handleCompletion(trailers: [:])
                }
            }
            .onTrailers { [weak self] trailers in
                self?.handleCompletion(trailers: trailers)
            }
            .onError { [weak self] _ in
                // TODO: Switch to call a different handler with the error
                self?.handleCompletion(trailers: [:])
            }
    }

    // MARK: - Private

    private func handleHeaders(_ headers: [String: [String]]) {
        let grpcStatus = headers["grpc-status"]?
            .first
            .flatMap(Int.init)
        self.onHeaders?(headers, grpcStatus)
    }

    private func handleData(_ data: Data) {
        if data.isEmpty {
            return
        }

        if var existingBuffer = self.buffer {
            existingBuffer.append(contentsOf: data)
        } else {
            self.buffer = data
        }

        self.processBuffer()
    }

    private func handleCompletion(trailers: [String: [String]]) {
        self.onComplete?(trailers)
    }

    private func processBuffer() {
        assert(self.buffer != nil, "Buffer should never be nil when processing")

        switch self.state {
        case .expectingCompressionFlag:
            guard let compressionFlag: UInt8 = self.buffer.integer(at: 0) else {
                return
            }

            guard compressionFlag == 0 else {
                assertionFailure("gRPC decompression is not supported")
                self.buffer.removeAll()
                self.state = .expectingCompressionFlag
                return
            }

            self.buffer.removeFirst()
            self.state = .expectingMessageLength

        case .expectingMessageLength:
            guard let messageLength: UInt32 = self.buffer.integer(at: 0) else {
                return
            }

            self.buffer.removeFirst(MemoryLayout<UInt32>.size)
            self.state = .expectingMessage(length: messageLength)

        case .expectingMessage(let length):
            let lengthAsInt = Int(length)
            self.onMessage?(self.buffer.withUnsafeBytes { rawBufferPointer -> Data in
                return Data(bytes: rawBufferPointer.baseAddress!, count: lengthAsInt)
            })

            self.buffer.removeFirst(lengthAsInt)
            self.state = .expectingCompressionFlag
        }

        self.processBuffer()
    }
}

private extension Data {
    func integer<T: FixedWidthInteger>(at index: Int) -> T? {
        return self.withUnsafeBytes { bufferPointer -> T? in
            guard index <= bufferPointer.count - MemoryLayout<T>.size else {
                return nil
            }

            var value: T = 0
            Swift.withUnsafeMutableBytes(of: &value) { valuePointer in
                valuePointer.copyMemory(from: UnsafeRawBufferPointer(
                    start: bufferPointer.baseAddress!.advanced(by: index),
                    count: MemoryLayout<T>.size))
            }

            return value.bigEndian
        }
    }
}

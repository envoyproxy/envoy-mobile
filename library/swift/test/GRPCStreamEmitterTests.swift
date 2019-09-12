@testable import Envoy
import Foundation
import XCTest

private let kMessageData = Data([1, 2, 3, 4])

private final class MockEmitter: StreamEmitter {
  private let onSendData: (Data) -> Void

  init(onSendData: @escaping (Data) -> Void) {
    self.onSendData = onSendData
  }

  func sendData(_ data: Data) -> StreamEmitter {
    self.onSendData(data)
    return self
  }

  func sendMetadata(_ metadata: [String: [String]]) -> StreamEmitter {
    return self
  }

  func close(trailers: [String: [String]]) {}
  func cancel() {}
}

final class GRPCStreamEmitterTests: XCTestCase {
  func testDataSizeIsFiveBytesGreaterThanMessageSize() {
    var sentData = Data()
    let mockEmitter = MockEmitter(onSendData: { sentData.append(contentsOf: $0) })
    let grpcEmitter = GRPCStreamEmitter(emitter: mockEmitter)
    grpcEmitter.sendMessage(kMessageData)
    XCTAssertEqual(5 + kMessageData.count, sentData.count)
  }

  func testPrefixesSentDataWithZeroCompressionFlag() {
    var sentData = Data()
    let mockEmitter = MockEmitter(onSendData: { sentData.append(contentsOf: $0) })
    let grpcEmitter = GRPCStreamEmitter(emitter: mockEmitter)
    grpcEmitter.sendMessage(kMessageData)
    XCTAssertEqual(UInt8(0), sentData.integer(atIndex: 0))
  }

  func testPrefixesSentDataWithLengthOfMessage() {
    var sentData = Data()
    let mockEmitter = MockEmitter(onSendData: { sentData.append(contentsOf: $0) })
    let grpcEmitter = GRPCStreamEmitter(emitter: mockEmitter)
    grpcEmitter.sendMessage(kMessageData)
    let messageSize: UInt32? = sentData.integer(atIndex: 1)
    XCTAssertEqual(UInt32(kMessageData.count).bigEndian, messageSize)
  }

  func testAppendsMessageDataAtTheEndOfSentData() {
    var sentData = Data()
    let mockEmitter = MockEmitter(onSendData: { sentData.append(contentsOf: $0) })
    let grpcEmitter = GRPCStreamEmitter(emitter: mockEmitter)
    grpcEmitter.sendMessage(kMessageData)
    sentData.removeSubrange(0..<5) // Remove compression and length prefix
    XCTAssertEqual(kMessageData, sentData)
  }
}

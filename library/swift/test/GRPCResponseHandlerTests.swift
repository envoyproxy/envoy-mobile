@testable import Envoy
import Foundation
import XCTest

private let kMessage1 = Data([1, 2, 3, 4, 5])
private let kMessage2 = Data([6, 7, 8, 9, 0, 1])

final class GRPCResponseHandlerTests: XCTestCase {
  func testHeadersCallbackPassesTrailersAndGRPCStatus() {
    let expectation = self.expectation(description: "Closure is called")
    let expectedHeaders: [String : [String]] = ["grpc-status": ["1"], "other": ["foo", "bar"]]
    let handler = GRPCResponseHandler()
      .onHeaders { headers, grpcStatus, endStream in
        XCTAssertEqual(expectedHeaders, headers)
        XCTAssertEqual(1, grpcStatus)
        XCTAssertTrue(endStream)
        expectation.fulfill()
      }

    handler.underlyingHandler.underlyingCallbacks.onHeaders(expectedHeaders, true)
    self.waitForExpectations(timeout: 0.1)
  }

  func testTrailersCallbackPassesTrailers() {
    let expectation = self.expectation(description: "Closure is called")
    let expectedTrailers: [String : [String]] = ["foo": ["bar"], "baz": ["1", "2"]]
    let handler = GRPCResponseHandler()
      .onTrailers { trailers in
        XCTAssertEqual(expectedTrailers, trailers)
        expectation.fulfill()
      }

    handler.underlyingHandler.underlyingCallbacks.onTrailers(expectedTrailers)
    self.waitForExpectations(timeout: 0.1)
  }

  func testMessageCallbackBuffersDataSentInSingleChunk() {
    let expectation = self.expectation(description: "Closure is called")
    let firstMessage = Data([
      0x0, // Compression flag
      0x5, 0x0, 0x0, 0x0, // Length bytes
    ] + kMessage1)

    let handler = GRPCResponseHandler()
      .onMessage { message, endStream in
        XCTAssertEqual(kMessage1, message)
        XCTAssertTrue(endStream)
        expectation.fulfill()
      }

    handler.underlyingHandler.underlyingCallbacks.onData(firstMessage, true)
    self.waitForExpectations(timeout: 0.1)
  }

  func testMessageCallbackBuffersDataSentInMultipleChunks() {
    let expectation = self.expectation(description: "Closure is called")
    expectation.expectedFulfillmentCount = 2

    let firstMessage = Data([
      0x0, // Compression flag
      0x5, 0x0, 0x0, 0x0, // Length bytes
    ] + kMessage1)

    let secondMessagePart1 = Data([
      0x0, // Compression flag
      0x6, 0x0, 0x0, // 3/4 length bytes
    ])

    let secondMessagePart2 = Data([
      0x0, // Last length byte
    ] + kMessage2[0..<2])

    let secondMessagePart3 = Data(kMessage2[2..<6])

    var expectedMessages = [kMessage1, kMessage2]
    let handler = GRPCResponseHandler()
      .onMessage { message, endStream in
        XCTAssertEqual(expectedMessages.removeFirst(), message)
        XCTAssertEqual(expectedMessages.isEmpty, endStream)
        expectation.fulfill()
      }

    handler.underlyingHandler.underlyingCallbacks.onData(firstMessage, false)
    handler.underlyingHandler.underlyingCallbacks.onData(secondMessagePart1, false)
    handler.underlyingHandler.underlyingCallbacks.onData(secondMessagePart2, false)
    handler.underlyingHandler.underlyingCallbacks.onData(secondMessagePart3, true)
    self.waitForExpectations(timeout: 0.1)
    XCTAssertTrue(expectedMessages.isEmpty)
  }

  // MARK: - gRPC status parsing

  func testParsingGRPCStatusFromHeadersReturnsFirstStatus() {
    let headers = [":status": ["200"], "grpc-status": ["1", "2"]]
    XCTAssertEqual(1, GRPCResponseHandler.grpcStatus(fromHeaders: headers))
  }

  func testParsingInvalidGRPCStatusReturnsZero() {
    let headers = ["grpc-status": ["invalid"]]
    XCTAssertEqual(0, GRPCResponseHandler.grpcStatus(fromHeaders: headers))
  }

  func testParsingMissingGRPCStatusReturnsZero() {
    XCTAssertEqual(0, GRPCResponseHandler.grpcStatus(fromHeaders: [:]))
  }
}

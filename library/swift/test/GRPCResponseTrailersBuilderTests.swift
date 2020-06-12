@testable import Envoy
import Foundation
import XCTest

final class GRPCResponseTrailersBuilderTests: XCTestCase {
  func testParsingGRPCStatusHeader() {
    let trailers = GRPCResponseTrailers(headers: ["grpc-status": ["7"]])
    XCTAssertEqual(7, trailers.grpcStatus)
  }

  func testUpdatingGRPCStatusHeader() {
    let trailers = GRPCResponseTrailers(headers: ["grpc-status": ["7"]])
      .toGRPCResponseTrailersBuilder()
      .addGrpcStatus(14)
      .build()

    XCTAssertEqual(14, trailers.grpcStatus)
  }

  func testParsingGRPCMessageHeader() {
    let trailers = GRPCResponseTrailers(headers: ["grpc-message": ["hello world"]])
    XCTAssertEqual("hello world", trailers.grpcMessage)
  }

  func testUpdatingGRPCMessageHeader() {
    let trailers = GRPCResponseTrailers(headers: ["grpc-message": ["abc"]])
      .toGRPCResponseTrailersBuilder()
      .addGrpcMessage("123")
      .build()

    XCTAssertEqual("123", trailers.grpcMessage)
  }
}

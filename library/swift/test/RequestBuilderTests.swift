import Envoy
import Foundation
import XCTest

// swiftlint:disable:next force_unwrapping
private let kURL = URL(string: "http://0.0.0.0:9001/api.lyft.com/demo.txt")!
private let kBodyData = Data([1, 2, 3, 4])
private let kRetryPolicy = RetryPolicy(maxRetryCount: 123,
                                       retryOn: [.connectFailure, .fiveXX],
                                       perRetryTimeoutMS: 9000)

final class RequestBuilderTests: XCTestCase {
  // MARK: - Method

  func testHasMatchingMethodPresentInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .build()
    XCTAssertEqual(.post, request.method)
  }

  // MARK: - URL

  func testHasMatchingURLPresentInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .build()
    XCTAssertEqual(kURL.absoluteString, request.url.absoluteString)
  }

  // MARK: - Body data

  func testAddingRequestDataHasBodyPresentInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addBody(kBodyData)
      .build()
    XCTAssertEqual(kBodyData, request.body)
  }

  func testNotAddingRequestDataHasNilBodyInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .build()
    XCTAssertNil(request.body)
  }

  // MARK: - Retry policy

  func testAddingRetryPolicyHasRetryPolicyInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addRetryPolicy(kRetryPolicy)
      .build()
    XCTAssertEqual(kRetryPolicy, request.retryPolicy)
  }

  func testNotAddingRetryPolicyHasNilRetryPolicyInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .build()
    XCTAssertNil(request.retryPolicy)
  }

  // MARK: - Headers

  func testAddingNewHeaderAddsToListOfHeaderKeys() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "bar")
      .build()
    XCTAssertEqual(["bar"], request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyRemovesAllOfItsValuesFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyDoesNotRemoveOtherKeysFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "bar", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertEqual(["bar": ["2"]], request.headers)
  }

  func testRemovingSpecificHeaderValueRemovesItFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .addHeader(name: "foo", value: "3")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertEqual(["1", "3"], request.headers["foo"])
  }

  func testRemovingAllHeaderValuesRemovesKeyFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeader(name: "foo", value: "1")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  // MARK: - Trailers

  func testAddingNewTrailerAppendsToListOfTrailerKeys() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "bar")
      .build()
    XCTAssertEqual(["bar"], request.trailers["foo"])
  }

  func testRemovingSpecificTrailerKeyRemovesAllOfItsValuesFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .removeTrailers(name: "foo")
      .build()
    XCTAssertNil(request.trailers["foo"])
  }

  func testRemovingSpecificTrailerKeyDoesNotRemoveOtherKeysFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "bar", value: "2")
      .removeTrailers(name: "foo")
      .build()
    XCTAssertEqual(["bar": ["2"]], request.trailers)
  }

  func testRemovingSpecificTrailerValueRemovesItFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .addTrailer(name: "foo", value: "3")
      .removeTrailer(name: "foo", value: "2")
      .build()
    XCTAssertEqual(["1", "3"], request.trailers["foo"])
  }

  func testRemovingAllTrailerValuesRemovesKeyFromRequest() throws {
    let request = try RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .removeTrailer(name: "foo", value: "1")
      .removeTrailer(name: "foo", value: "2")
      .build()
    XCTAssertNil(request.trailers["foo"])
  }

  // MARK: - Request conversion

  func testRequestsAreEqualWhenPropertiesAreEqual() throws {
    let request1 = try self.newRequestBuilder().build()
    let request2 = try self.newRequestBuilder().build()
    XCTAssertEqual(request1, request2)
  }

  func testPointersAreNotEqualWhenInstancesAreDifferent() throws {
    let request1 = try self.newRequestBuilder().build()
    let request2 = try self.newRequestBuilder().build()
    XCTAssert(request1 !== request2)
  }

  func testConvertingToBuilderAndBackMaintainsEquality() throws {
    let request1 = try self.newRequestBuilder().build()
    let request2 = request1.newBuilder().build()
    XCTAssertEqual(request1, request2)
  }

  // MARK: - Invalid headers/trailers

  func testThrowsWhenRestrictedHeaderIsAdded() {
    let builder = RequestBuilder(method: .post, url: kURL)
    XCTAssertThrowsError(try builder.addHeader(name: ":method", value: "POST")) { error in
      XCTAssertEqual(.restrictedHeader, error as? RequestBuilderError)
    }
  }

  func testThrowsWhenRestrictedTrailerIsAdded() {
    let builder = RequestBuilder(method: .post, url: kURL)
    XCTAssertThrowsError(try builder.addTrailer(name: ":method", value: "POST")) { error in
      XCTAssertEqual(.restrictedHeader, error as? RequestBuilderError)
    }
  }

  // MARK: - Private

  private func newRequestBuilder() throws -> RequestBuilder {
    return try RequestBuilder(method: .post, url: kURL)
      .addBody(kBodyData)
      .addRetryPolicy(kRetryPolicy)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .addHeader(name: "bar", value: "3")
      .addTrailer(name: "baz", value: "4")
      .addTrailer(name: "baz", value: "5")
  }
}

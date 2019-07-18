import Envoy
import Foundation
import XCTest

// swiftlint:disable:next force_unwrapping
private let kURL = URL(string: "http://0.0.0.0:9001/api.lyft.com/demo.txt")!
private let kBodyData = Data([1, 2, 3, 4])

private let kRetryPolicy1 = RetryPolicy(maxRetryCount: 123,
                                        retryOn: [.connectFailure, .fiveXX],
                                        perRetryTimeoutMS: 9000)

private let kRetryPolicy2 = RetryPolicy(maxRetryCount: 123,
                                        retryOn: [.connectFailure, .fiveXX],
                                        perRetryTimeoutMS: 9000)

private let kRequestBuilder1 = RequestBuilder(method: .post, url: kURL)
  .addBody(kBodyData)
  .addRetryPolicy(kRetryPolicy1)
  .addHeader(name: "foo", value: "1")
  .addHeader(name: "foo", value: "2")
  .addHeader(name: "bar", value: "3")
  .addTrailer(name: "baz", value: "4")
  .addTrailer(name: "baz", value: "5")

private let kRequestBuilder2 = RequestBuilder(method: .post, url: kURL)
  .addBody(kBodyData)
  .addRetryPolicy(kRetryPolicy2)
  .addHeader(name: "foo", value: "1")
  .addHeader(name: "foo", value: "2")
  .addHeader(name: "bar", value: "3")
  .addTrailer(name: "baz", value: "4")
  .addTrailer(name: "baz", value: "5")

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
      .addRetryPolicy(kRetryPolicy1)
      .build()
    XCTAssertEqual(kRetryPolicy1, request.retryPolicy)
  }

  func testNotAddingRetryPolicyHasNilRetryPolicyInRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .build()
    XCTAssertNil(request.retryPolicy)
  }

  // MARK: - Headers

  func testAddingNewHeaderAddsToListOfHeaderKeys() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "bar")
      .build()
    XCTAssertEqual(["bar"], request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyRemovesAllOfItsValuesFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyDoesNotRemoveOtherKeysFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "bar", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertEqual(["bar": ["2"]], request.headers)
  }

  func testRemovingSpecificHeaderValueRemovesItFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .addHeader(name: "foo", value: "3")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertEqual(["1", "3"], request.headers["foo"])
  }

  func testRemovingAllHeaderValuesRemovesKeyFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeader(name: "foo", value: "1")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  // MARK: - Trailers

  func testAddingNewTrailerAppendsToListOfTrailerKeys() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "bar")
      .build()
    XCTAssertEqual(["bar"], request.trailers["foo"])
  }

  func testRemovingSpecificTrailerKeyRemovesAllOfItsValuesFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .removeTrailers(name: "foo")
      .build()
    XCTAssertNil(request.trailers["foo"])
  }

  func testRemovingSpecificTrailerKeyDoesNotRemoveOtherKeysFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "bar", value: "2")
      .removeTrailers(name: "foo")
      .build()
    XCTAssertEqual(["bar": ["2"]], request.trailers)
  }

  func testRemovingSpecificTrailerValueRemovesItFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .addTrailer(name: "foo", value: "3")
      .removeTrailer(name: "foo", value: "2")
      .build()
    XCTAssertEqual(["1", "3"], request.trailers["foo"])
  }

  func testRemovingAllTrailerValuesRemovesKeyFromRequest() {
    let request = RequestBuilder(method: .post, url: kURL)
      .addTrailer(name: "foo", value: "1")
      .addTrailer(name: "foo", value: "2")
      .removeTrailer(name: "foo", value: "1")
      .removeTrailer(name: "foo", value: "2")
      .build()
    XCTAssertNil(request.trailers["foo"])
  }

  // MARK: - Request conversion

  func testRequestsAreEqualWhenPropertiesAreEqual() {
    let request1 = kRequestBuilder1.build()
    let request2 = kRequestBuilder2.build()
    XCTAssertEqual(request1, request2)
  }

  func testPointersAreNotEqualWhenInstancesAreDifferent() {
    let request1 = kRequestBuilder1.build()
    let request2 = request1.newBuilder().build()
    XCTAssertEqual(request1, request2)
  }
}

import Envoy
import Foundation
import XCTest

private let kRetryPolicy = RetryPolicy(maxRetryCount: 123,
                                       retryOn: [.connectFailure, .status5xx],
                                       perRetryTimeoutMS: 9000)

final class RequestBuilderTests: XCTestCase {
  // MARK: - Method

  func testHasMatchingMethodPresentInRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .build()
    XCTAssertEqual(.post, request.method)
  }

  // MARK: - URL

  func testHasMatchingURLPresentInRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .build()
    XCTAssertEqual("https", request.scheme)
    XCTAssertEqual("api.foo.com", request.authority)
    XCTAssertEqual("/foo", request.path)
  }

  // MARK: - Retry policy

  func testAddingRetryPolicyHasRetryPolicyInRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addRetryPolicy(kRetryPolicy)
      .build()
    XCTAssertEqual(kRetryPolicy, request.retryPolicy)
  }

  func testNotAddingRetryPolicyHasNilRetryPolicyInRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .build()
    XCTAssertNil(request.retryPolicy)
  }

  // MARK: - Headers

  func testAddingNewHeaderAddsToListOfHeaderKeys() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addHeader(name: "foo", value: "bar")
      .build()
    XCTAssertEqual(["bar"], request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyRemovesAllOfItsValuesFromRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  func testRemovingSpecificHeaderKeyDoesNotRemoveOtherKeysFromRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "bar", value: "2")
      .removeHeaders(name: "foo")
      .build()
    XCTAssertEqual(["bar": ["2"]], request.headers)
  }

  func testRemovingSpecificHeaderValueRemovesItFromRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .addHeader(name: "foo", value: "3")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertEqual(["1", "3"], request.headers["foo"])
  }

  func testRemovingAllHeaderValuesRemovesKeyFromRequest() {
    let request = RequestBuilder(method: .post, scheme: "https",
                                 authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .removeHeader(name: "foo", value: "1")
      .removeHeader(name: "foo", value: "2")
      .build()
    XCTAssertNil(request.headers["foo"])
  }

  // MARK: - Request conversion

  func testRequestsAreEqualWhenPropertiesAreEqual() {
    let request1 = self.newRequestBuilder().build()
    let request2 = self.newRequestBuilder().build()
    XCTAssertEqual(request1, request2)
  }

  func testPointersAreNotEqualWhenInstancesAreDifferent() {
    let request1 = self.newRequestBuilder().build()
    let request2 = self.newRequestBuilder().build()
    XCTAssert(request1 !== request2)
  }

  func testConvertingToBuilderAndBackMaintainsEquality() {
    let request1 = self.newRequestBuilder().build()
    let request2 = request1.newBuilder().build()
    XCTAssertEqual(request1, request2)
  }

  // MARK: - Private

  private func newRequestBuilder() -> RequestBuilder {
    return RequestBuilder(method: .post, scheme: "https", authority: "api.foo.com", path: "/foo", upstreamHttpProtocol: .http2)
      .addRetryPolicy(kRetryPolicy)
      .addHeader(name: "foo", value: "1")
      .addHeader(name: "foo", value: "2")
      .addHeader(name: "bar", value: "3")
  }
}

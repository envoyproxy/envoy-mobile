import Envoy
import XCTest

private final class MockHeaderMutationFilter: RequestFilter {
  private let headersToAdd: [String: String]

  init(headersToAdd: [String: String]) {
    self.headersToAdd = headersToAdd
  }

  func onRequestHeaders(_ headers: RequestHeaders, endStream: Bool)
    -> FilterHeadersStatus<RequestHeaders>
  {
    let builder = headers.toRequestHeadersBuilder()
    for (name, value) in self.headersToAdd {
      builder.add(name: name, value: value)
    }
    return .continue(headers: builder.build())
  }

  func onRequestData(_ body: Data, endStream: Bool) -> FilterDataStatus<RequestHeaders> {
    return .continue(data: body)
  }

  func onRequestTrailers(_ trailers: RequestTrailers)
    -> FilterTrailersStatus<RequestHeaders, RequestTrailers>
  {
    return .continue(trailers: trailers)
  }
}

final class DirectResponseTests: XCTestCase {
  func testDirectResponseWithPrefixMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/foo/bar?param=1"
    ).build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(matcher: RouteMatcher(pathPrefix: "/v1/foo"), status: 200, body: "hello world")
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithExactMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    ).build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(matcher: RouteMatcher(fullPath: "/v1/abc"), status: 200, body: "hello world")
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithExactHeadersMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    )
    .add(name: "x-foo", value: "123")
    .build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(
          matcher: RouteMatcher(
            fullPath: "/v1/abc", headers: [
              .init(name: "x-foo", value: "123", mode: .exact)
            ]
          ),
          status: 200, body: "hello world"
        )
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithContainsHeadersMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    )
    .add(name: "x-foo", value: "123")
    .add(name: "x-foo", value: "456")
    .build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(
          matcher: RouteMatcher(
            fullPath: "/v1/abc", headers: [
              .init(name: "x-foo", value: "123", mode: .contains)
            ]
          ),
          status: 200, body: "hello world"
        )
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithPrefixHeadersMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    )
    .add(name: "x-foo", value: "123456")
    .build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(
          matcher: RouteMatcher(
            fullPath: "/v1/abc", headers: [
              .init(name: "x-foo", value: "123", mode: .prefix)
            ]
          ),
          status: 200, body: "hello world"
        )
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithSuffixHeadersMatch() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    )
    .add(name: "x-foo", value: "123456")
    .build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(
          matcher: RouteMatcher(
            fullPath: "/v1/abc", headers: [
              .init(name: "x-foo", value: "456", mode: .suffix)
            ]
          ),
          status: 200, body: "hello world"
        )
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseThatOnlyMatchesWhenUsingHeadersAddedByFilter() {
    let headersExpectation = self.expectation(description: "Response headers received")
    let dataExpectation = self.expectation(description: "Response data received")

    let requestHeaders = RequestHeadersBuilder(
      method: .get, authority: "127.0.0.1", path: "/v1/abc"
    )
    .build()

    // This test validates that Envoy is able to properly route direct responses when a filter
    // mutates the outbound request in a way that makes it match one of the direct response
    // configurations (whereas if the filter was not present in the chain, the request would not
    // match any configurations). This behavior is provided by the C++ `RouteCacheResetFilter`.
    let engine = TestEngineBuilder()
      .addPlatformFilter { MockHeaderMutationFilter(headersToAdd: ["x-foo": "123"]) }
      .addDirectResponse(
        .init(
          matcher: RouteMatcher(
            fullPath: "/v1/abc", headers: [
              .init(name: "x-foo", value: "123", mode: .exact)
            ]
          ),
          status: 200, body: "hello world"
        )
      )
      .build()

    var responseBuffer = Data()
    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        responseBuffer.append(contentsOf: data)
        if endStream {
          XCTAssertEqual("hello world", String(data: responseBuffer, encoding: .utf8))
          dataExpectation.fulfill()
        }
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }
}

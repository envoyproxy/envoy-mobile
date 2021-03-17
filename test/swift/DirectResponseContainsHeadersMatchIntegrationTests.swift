import Envoy
import XCTest

final class DirectResponseContainsHeadersMatchIntegrationTests: XCTestCase {
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
              .init(name: "x-foo", value: "123", mode: .contains),
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
              .init(name: "x-foo", value: "123", mode: .prefix),
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
              .init(name: "x-foo", value: "456", mode: .suffix),
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
              .init(name: "x-foo", value: "123", mode: .exact),
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

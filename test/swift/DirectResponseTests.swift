import Envoy
import XCTest

private final class MockHeaderMutationFilter: RequestFilter {
  func onRequestHeaders(_ headers: RequestHeaders, endStream: Bool)
    -> FilterHeadersStatus<RequestHeaders>
  {
    let newHeaders = headers
      .toRequestHeadersBuilder()
      .add(name: "x-foo", value: "foobar")
      .build()
    return .continue(headers: newHeaders)
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
      method: .get, authority: "127.0.0.1", path: "/v1/foo"
    ).build()

    let engine = TestEngineBuilder()
      .addDirectResponse(
        .init(matcher: RouteMatcher(pathPrefix: "/v1/foo"), status: 200, body: "hello world")
      )
      .addStatsDomain("192.168.0.1")
      .build()

    engine
      .streamClient()
      .newStreamPrototype()
      .setOnResponseHeaders { headers, endStream in
        XCTAssertEqual(200, headers.httpStatus)
        XCTAssertFalse(endStream)
        headersExpectation.fulfill()
      }
      .setOnResponseData { data, endStream in
        guard endStream else {
          return
        }
        XCTAssertEqual("hello world", String(data: data, encoding: .utf8))
        dataExpectation.fulfill()
      }
      .start()
      .sendHeaders(requestHeaders, endStream: true)

    let expectations = [headersExpectation, dataExpectation]
    XCTAssertEqual(.completed, XCTWaiter().wait(for: expectations, timeout: 10, enforceOrder: true))
  }

  func testDirectResponseWithExactMatch() {

  }

  func testDirectResponseThatOnlyMatchesWhenUsingHeadersAddedByFilter() {

  }
}

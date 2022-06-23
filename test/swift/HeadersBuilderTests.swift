@testable import Envoy
import XCTest

private let kRetryPolicy = RetryPolicy(maxRetryCount: 123,
                                       retryOn: [.connectFailure, .status5xx],
                                       perRetryTimeoutMS: 9000)

final class HeadersBuilderTests: XCTestCase {
  func testAddingNewHeaderAddsToListOfHeaderKeys() {
    let headers = HeadersBuilder(headers: [:])
      .add(name: "x-foo", value: "1")
      .add(name: "x-foo", value: "2")
      .headers()
    XCTAssertEqual(["1", "2"], headers["x-foo"])
  }

  func testRemovingSpecificHeaderKeyRemovesAllOfItsValues() {
    let headers = HeadersBuilder(headers: [:])
      .add(name: "x-foo", value: "1")
      .add(name: "x-foo", value: "2")
      .remove(name: "x-foo")
      .headers()
    XCTAssertNil(headers["x-foo"])
  }

  func testRemovingSpecificHeaderKeyDoesNotRemoveOtherKeys() {
    let headers = HeadersBuilder(headers: [:])
      .add(name: "x-foo", value: "123")
      .add(name: "x-bar", value: "abc")
      .remove(name: "x-foo")
      .headers()
    XCTAssertEqual(["x-bar": ["abc"]], headers)
  }

  func testSettingHeaderReplacesExistingHeadersWithMatchingName() {
    let headers = HeadersBuilder(headers: [:])
      .add(name: "x-foo", value: "123")
      .set(name: "x-foo", value: ["abc"])
      .headers()
    XCTAssertEqual(["x-foo": ["abc"]], headers)
  }

  func testInitializationIsCaseInsensitivePreservesCasingAndProcessesConflictingHeadersInAlphabeticalOrder() {
    let headers = HeadersBuilder(headers: ["a": ["456"], "A": ["123"]])
    XCTAssertEqual(["A": ["123", "456"]], headers.headers())
  }

  func testAddingHeaderIsCaseInsensitiveAndHeaderCasingIsPreserved() {
    let headers = HeadersBuilder(headers: [:])
    headers.add(name: "fOo", value: "abc")
    headers.add(name: "foo", value: "123")
    XCTAssertEqual(["fOo": ["abc", "123"]], headers.headers())
  }

  func testSettingHeaderIsCaseInsensitiveAndHeaderCasingIsPreserved() {
    let headers = HeadersBuilder(headers: [:])
    headers.set(name: "foo", value: ["123"])
    headers.set(name: "fOo", value: ["abc"])
    XCTAssertEqual(["fOo": ["abc"]], headers.headers()) 
  }

  func testRemovingHeaderIsCaseInsensitive() {
    let headers = HeadersBuilder(headers: [:])
    headers.set(name: "foo", value: ["123"])
    headers.remove(name: "fOo")
    XCTAssertEqual([:], headers.headers()) 
  }

  func testRestrictedHeadersAreNotSettable() {
    let headers = RequestHeadersBuilder(method: .get, authority: "example.com", path: "/")
      .add(name: "host", value: "example.com")
      .set(name: ":scheme", value: ["http"])
      .set(name: ":path", value: ["/nope"])
      .headers()
    let expected = [
      ":authority": ["example.com"],
      ":path": ["/"],
      ":method": ["GET"],
      ":scheme": ["https"],
    ]
    XCTAssertEqual(expected, headers)
  }

  func testBuildersAreEqualIfUnderlyingHeadersAreEqual() {
    let builder1 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]])
    let builder2 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]])
    XCTAssertEqual(builder1, builder2)
  }

  func testHeadersAreEqualIfUnderlyingHeadersAreEqual() {
    let headers1 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]]).build()
    let headers2 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]]).build()
    XCTAssertEqual(headers1, headers2)
  }

  func testBuilderPointersAreNotEqualWhenInstancesAreDifferent() {
    let builder1 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]])
    let builder2 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]])
    XCTAssert(builder1 !== builder2)
  }

  func testHeadersPointersAreNotEqualWhenInstancesAreDifferent() {
    let headers1 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]]).build()
    let headers2 = RequestHeadersBuilder(headers: ["x-foo": ["123"], "x-bar": ["abc"]]).build()
    XCTAssert(headers1 !== headers2)
  }
}

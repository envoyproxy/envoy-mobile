@testable import Envoy
import XCTest

final class HeadersContainerTests: XCTestCase {
  func testInitializationPreservesAllHeadersFromInputHeadersMap() {
    let container = HeadersContainer(headers: ["a": ["456"], "b": ["123"]])
    XCTAssertEqual(["a": ["456"], "b": ["123"]], container.allHeaders())
  }

  func testInitializationIsCaseInsensitivePreservesCasingAndProcessesInAlphabeticalOrder() {
    let container = HeadersContainer(headers: ["a": ["456"], "A": ["123"]])
    XCTAssertEqual(["A": ["123", "456"]], container.allHeaders())
  }

  func testAddingHeaderValueAddsToListOfHeader() {
    var container =  HeadersContainer()
    container.add(name: "x-foo", value: "1")
    container.add(name: "x-foo", value: "2")

    XCTAssertEqual(["1", "2"], container.value(forName: "x-foo"))
  }

  func testAddingHeaderValueIsCaseInsensitiveAndPreservesHeaderNameCasing() {
    var container =  HeadersContainer()
    container.add(name: "x-FOO", value: "1")
    container.add(name: "x-foo", value: "2")

    XCTAssertEqual(["1", "2"], container.value(forName: "x-foo"))
    XCTAssertEqual(["x-FOO": ["1", "2"]], container.allHeaders())
  }

  func testAddingHeadersValuesAddsToListOfHeaders() {
    var container =  HeadersContainer()
    container.add(name: "x-foo", value: ["1", "2"])
    container.add(name: "x-foo", value: ["3", "4"])

    XCTAssertEqual(["1", "2", "3", "4"], container.value(forName: "x-foo"))
  }

  func testAddingHeaderValuesIsCaseInsensitiveAndPreservesHeaderNameCasing() {
    var container =  HeadersContainer()
    container.add(name: "x-FOO", value: ["1", "2"])
    container.add(name: "x-foo", value: ["3", "4"])

    XCTAssertEqual(["1", "2", "3", "4"], container.value(forName: "x-foo"))
    XCTAssertEqual(["x-FOO": ["1", "2", "3", "4"]], container.allHeaders())
  }

  func testSettingHeaderToNilRemovesAllOfItsValues() {
    var container = HeadersContainer()
    container.add(name: "x-foo", value: "1")
    container.add(name: "x-foo", value: "2")
    container.set(name: "x-foo", value: nil)

    XCTAssertNil(container.value(forName: "x-foo"))
  }

  func testSettingHeaderToNilPerformsCaseInsensitiveHeaderNameLookup() {
    var container =  HeadersContainer()
    container.add(name: "x-FOO", value: "1")
    container.add(name: "x-foo", value: "2")
    container.set(name: "x-foo", value: nil)

    XCTAssertNil(container.value(forName: "x-foo"))
  }

  func testLookupIsCaseInsensitive() {
    var container =  HeadersContainer()
    container.add(name: "x-FOO", value: "1")

    XCTAssertEqual(["1"], container.value(forName: "x-foo"))
    XCTAssertEqual(["1"], container.value(forName: "x-fOo"))
    XCTAssertEqual(["1"], container.value(forName: "x-FOO"))
  }
}
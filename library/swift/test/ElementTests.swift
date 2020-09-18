import Envoy
import XCTest

final class ElementTests: XCTestCase {
    func testElementNotEqualToObjectOfUnrelatedType() {
        let element = Element("foo")
        XCTAssertFalse(element.isEqual("bar"))
    }

    func testElementsEquality() {
      XCTAssertEqual(Element("foo"), Element("foo"))
      XCTAssertNotEqual(Element("foo"), Element("bar"))
    }
}

import Envoy
import Foundation
import XCTest

final class CounterTests: XCTestCase {
    func testConvenientMethodDelegatesToTheMainMethod() {
        class MockCounterImpl: Counter {
            var wasIncremented = false
            func increment(count: Int) {
                self.wasIncremented = true
            }
        }

        let counter = MockCounterImpl()
        counter.increment()
        XCTAssertTrue(counter.wasIncremented)
    }
}

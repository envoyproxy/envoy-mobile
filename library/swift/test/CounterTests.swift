import Envoy
import XCTest

final class CounterTests: XCTestCase {
    func testConvenientMethodDelegatesToTheMainMethod() {
        class MockCounterImpl: Counter {
            var count: Int?
            func increment(count: Int) -> Int32 {
                self.count = count
                return 0
            }
        }

        let counter = MockCounterImpl()
        XCTAssertEqual(0, counter.increment())
        XCTAssertEqual(1, counter.count)
    }
}

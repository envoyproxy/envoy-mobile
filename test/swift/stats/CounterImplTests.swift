@testable import Envoy
@testable import EnvoyEngine
import Foundation
import XCTest

final class CounterTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
    MockEnvoyEngine.onRecordCounter = nil
  }

  func testConvenientMethodDelegatesToTheMainMethod() {
    class MockCounterImpl: Counter {
      var count: Int?
      func increment(count: Int) {
        self.count = count
      }
      func attach(tags: [Tag]) -> Counter {
        return MockCounterImpl()
      }
    }

    let counter = MockCounterImpl()
    counter.increment()
    XCTAssertEqual(1, counter.count)
  }

  func testAttachReturnsNewCounter() {
    var actualSeries: String?
    var actualTags = [String: String]()
    var actualCount: UInt?
    MockEnvoyEngine.onRecordCounter = { series, tags, count in
        actualSeries = series
        actualTags = tags
        actualCount = count
    }
    let mockEngine = MockEnvoyEngine()
    let pulseClient = PulseClientImpl(engine: mockEngine)
    let counter = pulseClient.counter(
        elements: ["test", "stat"], tags: [Tag(key: "testKey", value: "testValue")])
    let counterNew = counter.attach(tags: [Tag(key: "testKey1", value: "testValue1")])
    counterNew.increment()
    XCTAssertEqual(actualSeries, "test.stat")
    XCTAssertEqual(actualTags, ["testKey1": "testValue1"])
    XCTAssertEqual(actualCount, 1)
  }
}

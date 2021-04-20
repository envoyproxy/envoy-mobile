@testable import Envoy
@testable import EnvoyEngine
import Foundation
import XCTest

final class TimerImplTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
    MockEnvoyEngine.onRecordHistogramDuration = nil
  }

  func testAttachReturnsNewTimer() {
    var actualSeries: String?
    var actualTags = [String: String]()
    var actualDuration: UInt?
    MockEnvoyEngine.onRecordHistogramDuration = { series, tags, duration in
        actualSeries = series
        actualTags = tags
        actualDuration = duration
    }
    let mockEngine = MockEnvoyEngine()
    let pulseClient = PulseClientImpl(engine: mockEngine)
    let timer = pulseClient.timer(
        elements: ["test", "stat"], tags: [Tag(key: "testKey", value: "testValue")])
    let timerNew = timer.attach(tags: [Tag(key: "testKey1", value: "testValue1")])

    timerNew.completeWithDuration(durationMs: 5)
    XCTAssertEqual(actualSeries, "test.stat")
    XCTAssertEqual(actualTags, ["testKey1": "testValue1"])
    XCTAssertEqual(actualDuration, 5)
  }
}

@testable import Envoy
@testable import EnvoyEngine
import Foundation
import XCTest

final class GaugeImplTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
    MockEnvoyEngine.onRecordGaugeSet = nil
  }

  func testAttachReturnsNewGauge() {
    var actualSeries: String?
    var actualTags = [String: String]()
    var actualValue: UInt?
    MockEnvoyEngine.onRecordGaugeSet = { series, tags, value in
        actualSeries = series
        actualTags = tags
        actualValue = value
    }
    let mockEngine = MockEnvoyEngine()
    let pulseClient = PulseClientImpl(engine: mockEngine)
    let gauge = pulseClient.gauge(
        elements: ["test", "stat"], tags: [Tag(key: "testKey", value: "testValue")])
    let gaugeNew = gauge.attach(tags: [Tag(key: "testKey1", value: "testValue1")])

    gaugeNew.set(value: 5)
    XCTAssertEqual(actualSeries, "test.stat")
    XCTAssertEqual(actualTags, ["testKey1": "testValue1"])
    XCTAssertEqual(actualValue, 5)
  }
}

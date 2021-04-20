@testable import Envoy
@testable import EnvoyEngine
import Foundation
import XCTest

final class DistributionImplTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
    MockEnvoyEngine.onRecordHistogramValue = nil
  }

  func testAttachReturnsNewDistribution() {
    var actualSeries: String?
    var actualTags = [String: String]()
    var actualDuration: UInt?
    MockEnvoyEngine.onRecordHistogramValue = { series, tags, duration in
        actualSeries = series
        actualTags = tags
        actualDuration = duration
    }
    let mockEngine = MockEnvoyEngine()
    let pulseClient = PulseClientImpl(engine: mockEngine)
    let distribution = pulseClient.distribution(
        elements: ["test", "stat"], tags: [Tag(key: "testKey", value: "testValue")])
    let distributionNew =
      distribution.attach(tags: [Tag(key: "testKey1", value: "testValue1")])

    distributionNew.recordValue(value: 5)
    XCTAssertEqual(actualSeries, "test.stat")
    XCTAssertEqual(actualTags, ["testKey1": "testValue1"])
    XCTAssertEqual(actualDuration, 5)
  }
}

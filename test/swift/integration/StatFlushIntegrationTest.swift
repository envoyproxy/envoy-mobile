import Envoy
import EnvoyEngine
import Foundation
import XCTest

final class StatFlushIntegrationTest: XCTestCase {
  func testLotsOfFlushesWithHistograms() throws {
    let loggingExpectation = self.expectation(description: "Run used platform logger")

    let client = try EngineBuilder()
      .addLogLevel(.debug)
      .addStatsFlushSeconds(1)
      .setLogger { msg in
        if msg.contains("starting main dispatch loop") {
          loggingExpectation.fulfill()
        }
      }
      .build()
      .streamClient()

    XCTAssertEqual(XCTWaiter.wait(for: [engineExpectation], timeout: 1), .completed)

    let distribution = pulseClient.distribution(elements: ["foo", "bar", "distribution"])

    distribution.recordValue(100)

    for i in 100 {
        client.flushStats()
    }
  }
}

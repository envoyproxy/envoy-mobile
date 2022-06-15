import Envoy
import Foundation
import XCTest

final class EngineApiTest: XCTestCase {
  func testEngineApis() throws {
    let engineExpectation = self.expectation(description: "Engine Running")

    let engine = EngineBuilder()
      .addLogLevel(.debug)
      .addStatsFlushSeconds(1)
      .setOnEngineRunning { engineHandle in
        // TODO(jpsim): Update this check when the engine singleton is removed
        XCTAssertEqual(engineHandle, 1)
        engineExpectation.fulfill()
      }
      .build()

    XCTAssertEqual(XCTWaiter.wait(for: [engineExpectation], timeout: 10), .completed)

    let pulseClient = engine.pulseClient()
    pulseClient.gauge(elements: ["foo", "bar"]).set(value: 1)

    XCTAssertTrue(engine.dumpStats().contains("foo.bar: 1"))

    engine.terminate()
  }
}

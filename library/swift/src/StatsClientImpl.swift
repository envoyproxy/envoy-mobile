@_implementationOnly import EnvoyEngine
import Foundation

/// Envoy implementation of StatsClient.
@objcMembers
final class StatsClientImpl: NSObject {
  private let engine: EnvoyEngine

  init(engine: EnvoyEngine) {
    self.engine = engine
    super.init()
  }
}

extension StatsClientImpl: StatsClient {
  func getCounter(elements: [String]) -> Counter {
    return Counter(elements: elements, engine: self.engine)
  }
}

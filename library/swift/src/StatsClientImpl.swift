import Foundation

/// Envoy implementation of StatsClient.
@objcMembers
final class StatsClient: NSObject {
  private let engine: EnvoyEngine

  init(engine: EnvoyEngine) {
    self.engine = engine
    super.init()
  }
}

extension StatsClientImpl: StatsClient {
  func getCounter(elements: [String]) -> Counter {
    return Counter(engine, elements)
  }
}

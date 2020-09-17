@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of time series counter.
@objcMembers
class CounterImpl: NSObject, Counter {
  private let series: String
  private weak var engine: EnvoyEngine?

  init(elements: [Element], engine: EnvoyEngine) {
    self.series = elements.map { $0.value }.joined(separator: ".")
    self.engine = engine
    super.init()
  }

  /// Increment the counter by the given count.
  func increment(count: Int) -> Int32 {
    guard let engine = self.engine else {
      return kEnvoyFailure
    }

    return engine.recordCounter(self.series, count: numericCast(count))
  }
}

@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of time series counter.
@objcMembers
final class CounterImpl: NSObject, Counter {
  private let series: String
  private weak var engine: EnvoyEngine?

  init(elements: [Element], engine: EnvoyEngine) {
    self.series = elements.map { $0.value }.joined(separator: ".")
    self.engine = engine
    super.init()
  }

  /// Increment the counter by the given count.
  func increment(count: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordCounterInc(self.series, count: numericCast(count))
  }
}

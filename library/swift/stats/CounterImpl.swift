@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of time series counter.
@objcMembers
final class CounterImpl: NSObject, Counter {
  private let series: String
  private let tags: [String: String]
  private weak var engine: EnvoyEngine?

  init(elements: [Element], tags: [Tag], engine: EnvoyEngine) {
    self.series = elements.map { $0.value }.joined(separator: ".")
    self.engine = engine
    self.tags = Stats.convert(tags: tags)
    super.init()
  }

  init(series: String, tags: [Tag], engine: EnvoyEngine?) {
    self.series = series
    self.engine = engine
    self.tags = Stats.convert(tags: tags)
    super.init()
  }

  /// Increment the counter by the given count.
  func increment(count: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordCounterInc(self.series, tags: self.tags, count: numericCast(count))
  }

  func attach(tags: [Tag]) -> Counter {
    return CounterImpl(series: self.series, tags: tags, engine: self.engine)
  }
}

@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of time series gauge.
@objcMembers
final class GaugeImpl: NSObject, Gauge {
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

  /// Set the gauge with the given value.
  func set(value: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordGaugeSet(self.series, tags: self.tags, value: numericCast(value))
  }

  /// Add the given amount to the gauge.
  func add(amount: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordGaugeAdd(self.series, tags: self.tags, amount: numericCast(amount))
  }

  /// Subtract the given amount from the gauge.
  func sub(amount: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordGaugeSub(self.series, tags: self.tags, amount: numericCast(amount))
  }

  func attach(tags: [Tag]) -> Gauge {
    return GaugeImpl(series: self.series, tags: tags, engine: self.engine)
  }
}

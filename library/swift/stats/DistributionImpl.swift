@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of distribution tracking quantile/sum/average stats
@objcMembers
final class DistributionImpl: NSObject, Distribution {
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

  /// Record a new int value for the distribution.
  func recordValue(value: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordHistogramValue(self.series, tags: tags, value: numericCast(value))
  }

  func attach(tags: [Tag]) -> Distribution {
    return DistributionImpl(series: self.series, tags: tags, engine: self.engine)
  }
}

@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of timer that can be used to track a distribution of time durations
@objcMembers
final class TimerImpl: NSObject, Timer {
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

  /// Record a new duration value for the distribution.
  func completeWithDuration(durationMs: Int) {
    // TODO(jingwei99) potentially surface error up if engine is nil.
    self.engine?.recordHistogramDuration(
      self.series, tags: tags, durationMs: numericCast(durationMs))
  }

  func attach(tags: [Tag]) -> Timer {
    return TimerImpl(series: self.series, tags: tags, engine: self.engine)
  }
}

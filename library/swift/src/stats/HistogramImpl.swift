@_implementationOnly import EnvoyEngine
import Foundation

/// The implementation of time series gauge.
@objcMembers
class HistogramImpl: NSObject, Gauge {
  private let series: String
  private weak var engine: EnvoyEngine?

  init(elements: [Element], engine: EnvoyEngine) {
    self.series = elements.map { $0.value }.joined(separator: ".")
    self.engine = engine
    super.init()
  }

  /// Record a new value for the histogram.
  /// TODO: potentially raise error to platform if the operation is not successful.
  func record(value: Int) {
    guard let engine = self.engine else {
      return
    }

    engine.recordHistogramDurationMs(self.series, value: numericCast(value))
  }
}

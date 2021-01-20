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

  /// Record a new value for the histogram distribution.
  /// TODO: potentially raise error to platform if the operation is not successful.
  func recordValue(value: Int) {
    guard let engine = self.engine else {
      return
    }

    engine.recordHistogram(self.series, value: numericCast(value))
  }
}

import Foundation

/// A time series distribution of duration measurements.
@objc
public protocol Timer: AnyObject {
  /// Record a new duration to add to the timer histogram distribution.
  func recordDuration(durationMs: Int)
}

import Foundation

/// A time series distribution of duration measurements.
@objc
public protocol Timer: AnyObject {
  /// Record a new duration to add to the timer.
  func completeWithDuration(durationMs: Int)

  /// Gets a timer with tags attached.
  func attach(tags: [Tag]) -> Timer
}

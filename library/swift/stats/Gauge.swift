import Foundation

/// A time series gauge.
@objc
public protocol Gauge: AnyObject {
  /// Set the gauge with the given value.
  func set(value: Int)

  /// Add the given amount to the gauge.
  func add(amount: Int)

  /// Subtract the given amount from the gauge.
  func sub(amount: Int)

  /// Gets a gauge with tags attached.
  func attach(tags: [Tag]) -> Gauge
}

import Foundation

/// A time series distribution tracking quantile/sum/average stats.
@objc
public protocol Distribution: AnyObject {
  /// Record a new value to add to the integer distribution.
  func recordValue(value: Int)

  /// Gets a distribution with tags attached.
  func attach(tags: [Tag]) -> Distribution
}

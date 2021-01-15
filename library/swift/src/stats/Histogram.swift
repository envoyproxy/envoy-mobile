import Foundation

/// A time series histogram.
@objc
public protocol Histogram: AnyObject {

  /// Record a new value to add to the histogram.
  func record(value: Int)
}
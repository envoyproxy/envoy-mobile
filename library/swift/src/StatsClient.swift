import Foundation

/// Client used to record time series metrics.
@objc
public protocol StatsClient: AnyObject {
  /// - parameter elements: Elements to identify a counter
  /// - returns: A Counter based on the joined elements.
  func counter(elements: [Element]) -> Counter
}

import Foundation

/// Client used to record timeseries metrics.
@objc
public protocol StatsClient: AnyObject {
  /// - returns: A Counter based on the joined elements.
  func getCounter(elements: [Element]) -> Counter
}

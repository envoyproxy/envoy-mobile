import Foundation

/// Engine represents a running instance of Envoy Mobile, and provides client interfaces that run on
/// that instnace.
@objc
public protocol Engine: AnyObject {
  /// - returns: A StreamClient for opening and managing HTTP streams
  func getStreamClient(): StreamClient

  /// - returns: A StatsClient for recording timeseries metrics.
  func getStatsClient(): StatsClient
}

import Foundation

/// Engine represents a running instance of Envoy Mobile, and provides client interfaces that run on
/// that instance.
@objc
public protocol Engine: AnyObject {
  /// - returns: A client for opening and managing HTTP streams.
  func streamClient() -> StreamClient

  /// A client for recording time series metrics.
  func pulseClient() -> PulseClient

  ///
  /// Flush the stats sinks outside of a flushing interval.
  /// Note: stats flushing may not be synchronous.
  /// Therefore, this function may return prior to flushing taking place.
  func flushStats()
}

import Foundation
@_implementationOnly import EnvoyEngine

/// Client for Envoy Mobile's stats library, Pulse, used to record client time series metrics.
///
/// Note: this an experimental interface and is subject to change. The implementation has not been
/// optimized, and there may be performance implications in production usage.
@objc
public protocol PulseClient: AnyObject {
  /// - parameter elements: Elements to identify a counter
  ///
  /// - returns: A Counter based on the joined elements.
  func counter(elements: [Element]) -> Counter

  /// - parameter elements: Elements to identify a gauge
  ///
  /// - returns: A Gauge based on the joined elements.
  func gauge(elements: [Element]) -> Gauge

  /// - parameter elements: Elements to identify a histogram
  ///
  /// - returns: A Histogram based on the joined elements that can be used as a timer
  func histogramTimer(elements: [Element]) -> Histogram

  /// - parameter elements: Elements to identify a histogram
  ///
  /// - returns: A Histogram based on the joined elements that is used for unspecified measurement units
  func histogramGeneric(elements: [Element]) -> Histogram
}

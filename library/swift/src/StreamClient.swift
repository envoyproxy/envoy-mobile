import Foundation

/// Client used to create new streams.
@objc
public protocol StreamClient: AnyObject {
  /// Create a new inactive stream which can be used to start active streams.
  ///
  /// - returns: The new inactive stream.
  func newStream() -> InactiveStream
}

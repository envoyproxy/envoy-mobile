import Foundation

/// Client used to create new streams.
@objc
public protocol StreamClient: AnyObject {
  /// Create a new stream prototype which can be used to start active streams.
  ///
  /// - returns: The new stream prototype.
  func newStream() -> StreamPrototype
}

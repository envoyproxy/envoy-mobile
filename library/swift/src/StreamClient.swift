/// Client used to create new streams.
public protocol StreamClient {
  /// Create a new inactive stream which can be used to start active streams.
  ///
  /// - returns: The new inactive stream.
  func newStream() -> InactiveStream
}

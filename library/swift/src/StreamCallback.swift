import Foundation

/// Callback interface for receiving stream events.
@objc
public protocol StreamCallback {
  /// Invoked when response headers are read.
  ///
  /// - parameter headers:    The headers of the response.
  /// - parameter statusCode: The status code of the response.
  func onHeaders(_ headers: [String: [String]], statusCode: Int)

  /// Invoked when response body is read.
  ///
  /// - parameter data:      Bytes in the response.
  /// - parameter endStream: True if the stream is complete.
  func onData(_ data: Data, endStream: Bool)

  /// Invoked when response metadata is read.
  ///
  /// - parameter metadata:  The metadata of the response.
  /// - parameter endStream: True if the stream is complete.
  func onMetadata(_ metadata: [String: [String]], endStream: Bool)

  /// Invoked when response trailers are read.
  ///
  /// - parameter trailers: The trailers of the response.
  func onTrailers(_ trailers: [String: [String]])

  /// Invoked when there is an internal Envoy error with the stream.
  ///
  /// - parameter error: The error that occurred with the stream.
  func onError(_ error: Error)

  /// Invoked when the stream is canceled.
  func onCanceled()

  /// Invoked when the stream is completed.
  func onCompletion()
}

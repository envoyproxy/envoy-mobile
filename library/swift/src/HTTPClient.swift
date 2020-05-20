import Foundation

/// Client that is able to send and receive HTTP requests.
@objc
public protocol HTTPClient {
  /// Start a new stream.
  ///
  /// - parameter headers: The request headers for opening a stream.
  /// - parameter queue:   Queue on which to receive callback events.
  ///
  /// - returns: Stream for sending and receiving data.
  func start(_ headers: RequestHeaders, queue: DispatchQueue) -> Stream

  /// Send a unary (non-streamed) request.
  ///
  /// Close with headers-only: Pass a nil body and nil trailers.
  /// Close with data:         Pass a body and nil trailers.
  /// Close with trailers:     Pass non-nil trailers.
  ///
  /// - parameter headers:  The request headers to send.
  /// - parameter body:     Data to send as the body.
  /// - parameter trailers: Trailers to send with the request.
  /// - parameter queue:    Queue on which to receive callback events.
  ///
  /// - returns: A cancelable request.
  @discardableResult
  func send(_ headers: RequestHeaders, body: Data?, trailers: RequestTrailers?,
            queue: DispatchQueue) -> Stream
  // TODO: Should this have a reduced interface that doesn't allow for sending more data?
}

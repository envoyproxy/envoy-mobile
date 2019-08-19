import Foundation

/// Callback interface for receiving stream events.
@objcMembers
public final class ResponseHandler: NSObject {
  /// Underlying observer which will be passed to the Envoy Engine.
  let underlyingObserver = EnvoyObserver()

  /// Dispatch queue upon which callbacks will be called.
  public let dispatchQueue: DispatchQueue

  public init(queue: DispatchQueue = .main) {
    self.dispatchQueue = queue
  }

  /// Specify a callback for when response headers are received by the stream.
  ///
  /// - parameter closure: Closure which will receive the headers, status code,
  ///                      and flag indicating if the stream is headers-only.
  @discardableResult
  public func onHeaders(_ closure:
    @escaping (_ headers: [String: [String]], _ statusCode: Int, _ endStream: Bool) -> Void)
    -> ResponseHandler
  {
    self.underlyingObserver.onHeaders = { headers, endStream in
      closure(headers, ResponseHandler.statusCode(fromHeaders: headers) ?? 0, endStream)
    }

    return self
  }

  /// Specify a callback for when a data frame is received by the stream.
  ///
  /// - parameter closure: Closure which will receive the data,
  ///                      and flag indicating if the stream is headers-only.
  @discardableResult
  public func onData(_ closure:
    @escaping (_ data: Data, _ endStream: Bool) -> Void)
    -> ResponseHandler
  {
    self.underlyingObserver.onData = closure
    return self
  }

  /// Specify a callback for when a data frame is received by the stream.
  ///
  /// - parameter closure: Closure which will receive the trailers.
  @discardableResult
  public func onTrailers(_ closure:
    @escaping (_ trailers: [String: [String]]) -> Void)
    -> ResponseHandler
  {
    self.underlyingObserver.onTrailers = closure
    return self
  }

  /// Specify a callback for when an internal Envoy exception occurs with the stream.
  ///
  /// - parameter closure: Closure which will be called when an error occurs.
  @discardableResult
  public func onError(_ closure:
    @escaping () -> Void)
    -> ResponseHandler
  {
    self.underlyingObserver.onError = closure
    return self
  }

  /// Specify a callback for when an internal Envoy exception occurs with the stream.
  ///
  /// - parameter closure: Closure which will be called when the stream is canceled.
  @discardableResult
  public func onCanceled(_ closure:
    @escaping () -> Void)
    -> ResponseHandler
  {
    self.underlyingObserver.onCancel = closure
    return self
  }

  /// Specify a callback for when an internal Envoy exception occurs with the stream.
  ///
  /// - parameter closure: Closure which will be called when the stream has been completed.
  @discardableResult
  public func onCompletion(_ closure:
    @escaping () -> Void)
    -> ResponseHandler
  {
    // TODO: onCompletion not available - do we need to call this manually?
    return self
  }

  // MARK: - Helpers

  static func statusCode(fromHeaders headers: [String: [String]]) -> Int? {
    return headers[":status"]?
      .compactMap(Int.init)
      .first
  }
}

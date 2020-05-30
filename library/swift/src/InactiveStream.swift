import Dispatch
@_implementationOnly import EnvoyEngine
import Foundation

/// A type representing a stream that has not yet been started.
///
/// Constructed via `StreamClient`, and used to assign response callbacks
/// prior to starting an `ActiveStream` by calling `start()`.
@objcMembers
public final class InactiveStream: NSObject {
  private let engine: EnvoyEngine
  private let callbacks = StreamCallbacks()

  /// Initialize a new instance of the inactive stream.
  ///
  /// - parameter engine: Engine to use for starting active streams.
  required init(engine: EnvoyEngine) {
    self.engine = engine
    super.init()
  }

  // MARK: - Public

  /// Start a new active stream.
  ///
  /// - parameter queue: Queue on which to receive callback events.
  ///
  /// - returns: The new active stream.
  public func start(queue: DispatchQueue = .main) -> ActiveStream {
    let engineCallbacks = EnvoyHTTPCallbacks(callbacks: self.callbacks, queue: queue)
    let engineStream = self.engine.startStream(with: engineCallbacks)
    return ActiveStream(underlyingStream: engineStream)
  }

  /// Specify a callback for when response headers are received by the stream.
  ///
  /// - parameter closure: Closure which will receive the headers
  ///                      and flag indicating if the stream is headers-only.
  ///
  /// - returns: This stream, for chaining syntax.
  @discardableResult
  public func setOnResponseHeaders(
    closure: @escaping (_ headers: ResponseHeaders, _ endStream: Bool) -> Void) -> InactiveStream
  {
    self.callbacks.onHeaders = closure
    return self
  }

  /// Specify a callback for when a data frame is received by the stream.
  /// If `endStream` is `true`, the stream is complete.
  ///
  /// - parameter closure: Closure which will receive the data
  ///                      and flag indicating whether this is the last data frame.
  ///
  /// - returns: This stream, for chaining syntax.
  @discardableResult
  public func setOnResponseData(
    closure: @escaping (_ body: Data, _ endStream: Bool) -> Void) -> InactiveStream
  {
    self.callbacks.onData = closure
    return self
  }

  /// Specify a callback for when trailers are received by the stream.
  /// If the closure is called, the stream is complete.
  ///
  /// - parameter closure: Closure which will receive the trailers.
  ///
  /// - returns: This stream, for chaining syntax.
  @discardableResult
  public func setOnResponseTrailers(
    closure: @escaping (_ trailers: ResponseTrailers) -> Void) -> InactiveStream
  {
    self.callbacks.onTrailers = closure
    return self
  }

  /// Specify a callback for when an internal Envoy exception occurs with the stream.
  /// If the closure is called, the stream is complete.
  ///
  /// - parameter closure: Closure which will be called when an error occurs.
  ///
  /// - returns: This stream, for chaining syntax.
  @discardableResult
  public func setOnError(
    closure: @escaping (_ error: EnvoyError) -> Void) -> InactiveStream
  {
    self.callbacks.onError = closure
    return self
  }

  /// Specify a callback for when the stream is canceled.
  /// If the closure is called, the stream is complete.
  ///
  /// - parameter closure: Closure which will be called when the stream is canceled.
  ///
  /// - returns: This stream, for chaining syntax.
  @discardableResult
  public func setOnCancel(
    closure: @escaping () -> Void) -> InactiveStream
  {
    self.callbacks.onCancel = closure
    return self
  }
}

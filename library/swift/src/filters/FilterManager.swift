//import Foundation
//
//final class FilterManager {
//  private let requestChain: [RequestFilter]
//  private let responseChain: [ResponseFilter]
//  private let underlyingStream: EnvoyHTTPStream
//  private let underlyingCallbacks: EnvoyHTTPCallbacks
//
//  /// <#function description#>
//  weak var responseHandler: ResponseHandler?
//
//  /// Initialize a new instance of the filter manager.
//  ///
//  /// - parameter underlyingStream:    <#underlyingStream description#>
//  /// - parameter underlyingCallbacks: <#underlyingCallbacks description#>
//  /// - parameter filterRegistry:      Filter registry to use for this stream.
//  init(underlyingStream: EnvoyHTTPStream, underlyingCallbacks: EnvoyHTTPCallbacks,
//       filterRegistry: FilterRegistry)
//  {
//    let allFilters = filterRegistry.createFilters()
//    self.requestChain = allFilters.compactMap { $0 as? RequestFilter }
//    self.responseChain = allFilters.reversed().compactMap { $0 as? ResponseFilter }
//    self.underlyingStream = underlyingStream
//    self.underlyingCallbacks = underlyingCallbacks
//
//    self.assignFilterCallbacks()
//    self.assignUnderlyingCallbacks()
//  }
//
//  private func assignFilterCallbacks() {
//    for requestFilter in self.requestChain {
//      requestFilter.setRequestFilterCallbacks(self)
//    }
//
//    for responseFilter in self.responseChain {
//      responseFilter.setResponseFilterCallbacks(self)
//    }
//  }
//
//  private func assignUnderlyingCallbacks() {
//    self.underlyingCallbacks.onHeaders = { headers, endStream in
//      // TODO: Send through filter chain
//      self.responseHandler?.onResponseHeaders(ResponseHeaders(headers: headers),
//                                              endStream: endStream)
//    }
//
//    self.underlyingCallbacks.onData = { data, endStream in
//      // TODO: Send through filter chain
//      self.responseHandler?.onResponseData(data, endStream: endStream)
//    }
//
//    self.underlyingCallbacks.onTrailers = { trailers in
//      // TODO: Send through filter chain
//      self.responseHandler?.onResponseTrailers(ResponseTrailers(headers: trailers))
//    }
//
//    self.underlyingCallbacks.onError = { errorCode, message, attemptCount in
//      // Note that the cast will return nil if attemptCount was negative
//      // This is the desired behavior because the bridge layer uses -1 to signify absence.
//      let error = EnvoyError(errorCode: errorCode, message: message,
//                             attemptCount: UInt32(exactly: attemptCount), cause: nil)
//      // TODO: Send through filter chain
//      self.responseHandler?.onError(error)
//    }
//
//    self.underlyingCallbacks.onCancel = {
//      // TODO: Send through filter chain
//      self.responseHandler?.onCancel()
//    }
//  }
//
//  // MARK: - Sending requests
//
//  /// Called once when the request is initiated.
//  ///
//  /// - parameter headers:   The current request headers.
//  /// - parameter endStream: Whether this is a headers-only request.
//  func sendHeaders(_ headers: RequestHeaders, endStream: Bool) {
//
//  }
//
//  /// Called any number of times whenever body data is sent.
//  ///
//  /// - parameter body:      The outbound body data chunk.
//  /// - parameter endStream: Whether this is the last data frame.
//  func sendData(_ body: Data, endStream: Bool) {
//
//  }
//
//  /// Called at most once when the request is closed from the client with trailers.
//  ///
//  /// - parameter trailers: The outbound trailers.
//  func sendRequestTrailers(_ trailers: RequestTrailers) {
//
//  }
//
//  /// Cancel and end the associated stream.
//  func cancel() {
//
//  }
//}
//
///// Interface for receiving responses on a stream.
///// Used by the `FilterManager` to relay responses after filtering.
//protocol ResponseHandler: AnyObject {
//  /// Called once when the response is initiated.
//  ///
//  /// - parameter headers:   The current response headers.
//  /// - parameter endStream: Whether this is a headers-only response.
//  func onResponseHeaders(_ headers: ResponseHeaders, endStream: Bool)
//
//  /// Called any number of times whenever body data is received.
//  ///
//  /// - parameter body:      The inbound body data chunk.
//  /// - parameter endStream: Whether this is the last data frame.
//  func onResponseData(_ body: Data, endStream: Bool)
//
//  /// Called at most once when the response is closed from the server with trailers.
//  ///
//  /// - parameter trailers: The outbound trailers.
//  func onResponseTrailers(_ trailers: ResponseTrailers)
//
//  /// Called at most once when an error within Envoy occurs.
//  ///
//  /// - error: The error that occurred within Envoy.
//  func onError(_ error: EnvoyError)
//
//  /// Called at most once when the client cancels the stream.
//  func onCancel()
//}
//
//extension FilterManager: RequestFilterCallbacks {
//  func continueRequest() {
//    fatalError()
//  }
//
//  func requestBuffer() -> Data? {
//    fatalError()
//  }
//
//  func addRequestTrailers(_ trailers: RequestTrailers) {
//    fatalError()
//  }
//}
//
//extension FilterManager: ResponseFilterCallbacks {
//  func continueResponse() {
//    fatalError()
//  }
//
//  func responseBuffer() -> Data? {
//    fatalError()
//  }
//
//  func addResponseTrailers(_ trailers: ResponseTrailers) {
//    fatalError()
//  }
//}
//
//extension Stream: ResponseHandler {
//  func onResponseHeaders(_ headers: ResponseHeaders, endStream: Bool) {
//    self.onHeadersCallback?(headers, endStream)
//  }
//
//  func onResponseData(_ body: Data, endStream: Bool) {
//    self.onDataCallback?(body, endStream)
//  }
//
//  func onResponseTrailers(_ trailers: ResponseTrailers) {
//    self.onTrailersCallback?(trailers)
//  }
//
//  func onError(_ error: EnvoyError) {
//    self.onErrorCallback?(error)
//  }
//
//  func onCancel() {
//    self.onCancelCallback?()
//  }
//}
//
//// Will the filter chain break if filters hold a strong reference to their filter manager (callbacks)?
////  - Yes, we can potentially solve this with a wrapper like what Envoy has which holds a weak reference
//// Since the client starts the request immediately, but consumers won't be able to specify callback closures until after the `Stream` object is returned, is it possible for them to miss synchronous completions?
//// I.e., the request filter chain will execute before the `Stream` object is returned to the consumer
//// Should the stream be created by the engine, or should the stream be started using a provided engine?
////  The former would be more testable
////
//// Engine should take the queue when starting the stream
//// What do we do with Cancelable stream?
//// How do we test this from the platform layer?

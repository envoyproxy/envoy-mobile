import Foundation

/// ResponseFilter supporting asynchronous resumption.
public protocol AsyncResponseFilter: ResponseFilter {
  /// Invoked explicitly in response to an asynchronous resumeResponse() callback when filter
  /// iteration has been stopped.
  ///
  /// - return: The resumption status including any HTTP entities that will be forwarded.
  func onResumeResponse(headers: ResponseHeaders?, data: Data?, trailers: ResponseTrailers?, endStream: Bool) -> FilterResumeStatus<ResponseHeaders, ResponseTrailers>

  /// Called by the filter manager once to initialize the filter callbacks that the filter should
  /// use.
  ///
  /// - parameter callbacks: The callbacks for this filter to use to interact with the chain.
  func setResponseFilterCallbacks(_ callbacks: ResponseFilterCallbacks)
}

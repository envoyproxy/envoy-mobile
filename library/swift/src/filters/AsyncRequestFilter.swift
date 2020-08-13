import Foundation

/// RequestFilter supporting asynchronous resumption.
public protocol AsyncRequestFilter: RequestFilter {
  /// Invoked explicitly in response to an asynchronous resumeRequest() callback when filter
  /// iteration has been stopped.
  ///
  /// @return: The resumption status including any previously held entities that remain
  ///          to be forwarded.
  ///
  func onResumeRequest() -> FilterResumeStatus<RequestHeaders, RequestTrailers>

  /// Called by the filter manager once to initialize the filter callbacks that the filter should
  /// use.
  ///
  /// - parameter callbacks: The callbacks for this filter to use to interact with the chain.
  func setRequestFilterCallbacks(_ callbacks: RequestFilterCallbacks)
}

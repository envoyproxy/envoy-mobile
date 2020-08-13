package io.envoyproxy.envoymobile

/*
 * RequestFilter supporting asynchronous resumption.
 */
interface AsyncRequestFilter : RequestFilter {
  /**
   * Invoked explicitly in response to an asynchronous resumeRequest() callback when filter
   * iteration has been stopped.
   *
   * @return: The resumption status including any previously held entities that remain
   *          to be forwarded.
   */
  fun onResumeRequest(): FilterResumeStatus<RequestHeaders, RequestTrailers>

  /**
   * Called by the filter manager once to initialize the filter callbacks that the filter should
   * use.
   *
   * @param callbacks: The callbacks for this filter to use to interact with the chain.
   */
   fun setRequestFilterCallbacks(callbacks: RequestFilterCallbacks)
}

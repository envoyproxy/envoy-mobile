package io.envoyproxy.envoymobile

/*
 * ResponseFilter supporting asynchronous resumption.
 */
interface AsyncResponseFilter : ResponseFilter {
  /**
   * Invoked explicitly in response to an asynchronous resumeResponse() callback when filter
   * iteration has been stopped.
   *
   * @return: The resumption status including any previously held entities that remain
   *          to be forwarded.
   */
  fun onResumeResponse(): FilterResumeStatus<ResponseHeaders, ResponseTrailers>

  /**
   * Called by the filter manager once to initialize the filter callbacks that the filter should
   * use.
   *
   * @param callbacks: The callbacks for this filter to use to interact with the chain.
   */
   fun setResponseFilterCallbacks(callbacks: ResponseFilterCallbacks)
}

package io.envoyproxy.envoymobile

/*
 * Status to be returned by filters when transmitting or receiving headers.
 */
sealed class FilterHeadersStatus<T : Headers> {
  /**
   * Continue filter chain iteration, passing the provided headers through.
   */
  class Continue<T : Headers>(val headers: T) : FilterHeadersStatus<T>()

  /**
   * Do not iterate to any of the remaining filters in the chain with headers.
   *
   * Returning `Continue` or `ResumeIteration from another filter invocation or calling
   * `resumeRequest()`/`resumeResponse()` MUST occur when continued filter iteration is
   * desired.
   */
  class StopIteration<T : Headers> : FilterHeadersStatus<T>()
}

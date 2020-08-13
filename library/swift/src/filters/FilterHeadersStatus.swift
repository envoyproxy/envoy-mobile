/// Status returned by filters when transmitting or receiving headers.
@frozen
public enum FilterHeadersStatus<T: Headers>: Equatable {
  /// Continue filter chain iteration, passing the provided headers through.
  case `continue`(T)

  /// Do not iterate to any of the remaining filters in the chain with headers.
  ///
  /// Returning `continue` or `resumeIteration` from another filter invocation or calling
  /// `resumeRequest()`/`resumeResponse()` MUST occur when continued filter iteration is
  /// desired.
  case stopIteration
}

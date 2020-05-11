/// Status returned by filters when transmitting or receiving trailers.
public enum FilterTrailerStatus<T: Headers> {
  /// Continue filter chain iteration, passing the provided trailers through.
  case `continue`(T)

  /// Do not iterate to any of the remaining filters in the chain with trailers.
  ///
  /// Calling `continueRequest()`/`continueResponse()` MUST occur when continued filter iteration
  /// is desired.
  case stopIteration(T)
}

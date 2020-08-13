/// Status returned by filters when transmitting or receiving trailers.
@frozen
public enum FilterTrailersStatus<T: Headers, U: Headers>: Equatable {
  /// Continue filter chain iteration, passing the provided trailers through.
  case `continue`(U)

  /// Do not iterate to any of the remaining filters in the chain with trailers.
  ///
  /// Because trailers are by definition the last HTTP entity of a request or response, only
  /// asynchronous filters support resumption after returning StopIteration from on*Trailers.
  /// Calling `resumeRequest()`/`resumeResponse()` MUST occur if continued filter iteration
  /// is desired.
  case stopIteration

  /**
   * Resume previously-stopped iteration, possibly forwarding headers and data, if iteration was
   * previously stopped during an on*Headers or on*Data invocation.
   *
   * It is an error to return ResumeIteration if iteration is not currently stopped, and it is
   * an error to include headers if headers have already been forwarded to the next filter
   * (i.e. iteration was stopped during an on*Data invocation instead of on*Headers).
   */
  case resumeIteration(headers: T? = nil, data: Data? = nil, trailers: U)
}

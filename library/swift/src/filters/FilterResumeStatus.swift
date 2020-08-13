/// Status returned by filters after resuming iteration asynchronously.
@frozen
public enum FilterResumeStatus<T: Headers, U: Headers>: Equatable {
  /**
   * Resume previously-stopped iteration, possibly forwarding headers and data, if iteration was
   * previously stopped during an on*Headers or on*Data invocation.
   *
   * It is an error to return resumeIteration if iteration is not currently stopped, and it is
   * an error to include headers if headers have already been forwarded to the next filter
   * (i.e. iteration was stopped during an on*Data invocation instead of on*Headers).
   */
  case resumeIteration(headers: T? = nil, data: Data? = nil, trailers: U?)
}

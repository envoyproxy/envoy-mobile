/*
 * Builder used for constructing instances of `RequestTrailers` type.
 */
class RequestTrailersBuilder: HeadersBuilder {
  /**
   * Build the request trailers using the current builder.
   *
   * @return RequestTrailers, New instance of request trailers.
   */
  fun build(): RequestTrailers {
    return RequestTrailers(headers)
  }
}

import Foundation

/// Builder used for constructing instances of `ResponseTrailers`.
@objcMembers
public final class ResponseTrailersBuilder: HeadersBuilder {
  /// Build the request trailers using the current builder.
  ///
  /// - returns: New instance of request trailers.
  public func build() -> ResponseTrailers {
    return ResponseTrailers(headers: self.headers)
  }
}

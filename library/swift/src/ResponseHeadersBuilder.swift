import Foundation

/// Builder used for constructing instances of `ResponseHeaders`.
@objcMembers
public final class ResponseHeadersBuilder: HeadersBuilder {
  /// Build the request headers using the current builder.
  ///
  /// - returns: New instance of request headers.
  public func build() -> ResponseHeaders {
    return ResponseHeaders(headers: self.headers)
  }
}

import Foundation

/// Allows for configuring Envoy to return a local response based on matching criteria.
/// Especially useful for testing/mocking clients.
/// https://www.envoyproxy.io/docs/envoy/latest/api-v3/config/route/v3/route_components.proto#config-route-v3-directresponseaction
@objcMembers
public final class DirectResponse: NSObject {
  public let matcher: RouteMatcher
  public let status: UInt
  public let body: String?

  /// Designated initializer.
  ///
  /// - parameter matcher: The matcher to use for returning a direct response.
  /// - parameter status:  This status code will be returned to the consumer.
  /// - parameter body:    A string that will be returned as the body of the response.
  public init(matcher: RouteMatcher, status: UInt, body: String?) {
    self.matcher = matcher
    self.status = status
    self.body = body
    super.init()
  }

  /// - returns: The representation of this template that can be populated in the engine's config.
  func resolvedYAMLFormat() -> String {
    let pathMatch: String
    if let fullPath = self.matcher.fullPath {
        pathMatch = "path: \"\(fullPath)\""
    } else if let pathPrefix = self.matcher.pathPrefix {
        pathMatch = "prefix: \"\(pathPrefix)\""
    } else {
        // Ideally we could use an enum with associated values to simplify this into
        // a single initializer and enforce this at compile time, but it is not
        // compatible with Objective-C.
        preconditionFailure("Unexpectedly allowed DirectResponse with no path matches")
    }

    let formattedHeaderMatches = self.matcher.headers.map { header in
      """
                          headers:
                            - name: "\(header.name)"
                              \(header.mode.resolvedYAMLFormat(value: header.value))
      """
    }.joined(separator: "\n")

    return
      """
                      - match:
                          \(pathMatch)
      \(formattedHeaderMatches)
                        direct_response:
                          status: \(self.status)
                          body: \(self.body.map { "{ inline_string: \"\($0)\" }" } ?? "")
      """
  }
}

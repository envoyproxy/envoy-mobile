import Foundation

/// Available upstream http protocols.
@objc
public enum UpstreamHttpProtocol: Int {
  case http1
  case http2

  /// String representation of the log level.
  var stringValue: String {
    switch self {
    case .http1:
      return "http1"
    case .http2:
      return "http2"
    }
  }
}

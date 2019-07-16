import Foundation

/// A result returned from Envoy.
@objcMembers
public final class Result: NSObject {
  /// The response returned from the server.
  public let response: Response?
  /// An error that was encountered on the network (i.e., going offline).
  public let error: NetworkError?

  /// Designated initializer.
  public init(response: Response?,
              error: NetworkError?)
  {
    self.response = response
    self.error = error
  }
}

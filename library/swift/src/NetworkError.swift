import Foundation

/// Error representing cases when no response was received from the server.
/// I.e., the client went offline or became disconnected.
@objcMembers
public final class NetworkError: NSObject, Swift.Error {
  /// Message describing this error.
  public let message: String?

  /// Designated initializer.
  public init(message: String?) {
    self.message = message
  }
}

import Foundation

@objcMembers
public final class EnvoyError: NSObject, Error {
  /// Error code associated with the exception that occurred.
  public let errorCode: Int
  /// A description of what exception that occurred.
  public let message: String
  /// Optional cause for the error.
  public let cause: Error?

  /// Creation of a stream failed within the Envoy engine.
  public static let streamCreationFailed = EnvoyError(
    errorCode: 1001,
    message: "Unable to start stream within Envoy",
    cause: nil)

  /// Sending through a stream failed within the Envoy engine.
  public static let streamSendFailed = EnvoyError(
    errorCode: 1002,
    message: "Unable to send over stream within Envoy",
    cause: nil)

  /// Closing a stream failed within the Envoy engine.
  public static let closeStreamFailed = EnvoyError(
    errorCode: 1003,
    message: "Unable to close stream within Envoy",
    cause: nil)

  init(errorCode: Int, message: String, cause: Swift.Error?) {
    self.errorCode = errorCode
    self.message = message
    self.cause = cause
  }
}

import Foundation

@objcMembers
public final class Error: NSObject, Swift.Error {
  /// Error code associated with the exception that occurred.
  public let code: Int
  /// A description of what exception that occurred.
  public let message: String
  /// Optional cause for the error.
  public let cause: Swift.Error?

  init(code: Int, message: String, cause: Swift.Error?) {
    self.code = code
    self.message = message
    self.cause = cause
  }
}

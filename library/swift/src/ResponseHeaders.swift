import Foundation

/// Headers representing an inbound response.
@objcMembers
public final class ResponseHeaders: Headers {
  /// HTTP status code received with the response.
  /// TODO: in Kotlin this blows up if the value of status cannot be translated to an int.
  public var httpStatus: Int? {
    return self.value(forName: ":status")?.first.flatMap(Int.init)
  }
}

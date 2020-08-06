import Foundation

fileprivate let pattern = "^[A-Za-z_]+$"

/// Element represents one dot-delimited component of a timeseries name.
@objc
public final class Element: NSObject, ExpressibleByStringLiteral {
  private let value: String

  public init(stringLiteral value: String) {
    guard value.range(of: pattern, options: .regularExpression) != nil else {
      preconditionFailure("Element values must conform to the regex /^[A-Za-z_]+$/.")
    }
    self.value = value
  }

  // CustomStringConvertible
  override public var description: String { return value }
}

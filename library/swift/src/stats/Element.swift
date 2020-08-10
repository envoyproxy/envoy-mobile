import Foundation

private let kPattern = "^[A-Za-z_]+$"

/// Element represents one dot-delimited component of a time series name.
/// Element values must conform to the regex /^[A-Za-z_]+$/.
@objc
public final class Element: NSObject, ExpressibleByStringLiteral {
  private let value: String

  public init(stringLiteral value: String) {
    guard value.range(of: kPattern, options: .regularExpression) != nil else {
      preconditionFailure("Element values must conform to the regex /^[A-Za-z_]+$/.")
    }
    self.value = value
  }

  // CustomStringConvertible
  public override var description: String { return value }
}

import Foundation

/// Element represents one dot-delimited component of a timeseries name.
public class Element: ExpressibleByStringLiteral {
  private static let pattern = "^[A-Za-z_]+$"
  private let value: String

  init?(string value: String) {
    guard value.range(of: pattern, options: .regularExpression) != nil else {
      return nil
    }
    self.value = value
  }
}

extension Element: ExpressibleByStringLiteral {
  convenience init(stringLiteral value: String) {
    self.init(string: value)! // Element values must conform to the regex /^[A-Za-z_]+$/.
  }
}

extension Element: CustomStringConveritble {
  public var description: String { return value }
}

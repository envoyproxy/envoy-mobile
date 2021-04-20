import Foundation

/// Tag represents the [key,value] component of a time series name.
/// Element values must conform to the regex /^[A-Za-z_]+$/.
@objcMembers
public final class Tag: NSObject {
  internal let key: String
  internal let value: String

  public init(key: String, value: String) {
    self.key = key
    self.value = value
  }

  public override func isEqual(_ object: Any?) -> Bool {
    return (object as? Tag)?.key == self.key && (object as? Tag)?.value == self.value
  }
}

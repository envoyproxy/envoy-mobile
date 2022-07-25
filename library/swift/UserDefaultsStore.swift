import Foundation

/// Simple implementation of a `KeyValueStore` leveraging `UserDefaults` for persistence.
@objcMembers
public final class UserDefaultsStore: KeyValueStore {
  private let defaults: UserDefaults

  public init(userDefaults: UserDefaults) {
    self.defaults = userDefaults
  }

  public func readValue(forKey key: String) -> String? {
    return self.defaults.string(forKey: key)
  }

  public func saveValue(_ value: String, toKey key: String) {
    self.defaults.set(value, forKey: key)
  }

  public func removeKey(_ key: String) {
    self.defaults.removeObject(forKey: key)
  }
}

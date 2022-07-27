

import Foundation

/// Simple implementation of a `KeyValueStore` leveraging `UserDefaults` for persistence.
@objcMembers
public class UserDefaultsStore: KeyValueStore {
  private let defaults: UserDefaults

  public init(userDefaults: UserDefaults) {
    self.defaults = userDefaults
  }

  public func readValue(forKey key: String) -> String? {
    return defaults.string(forKey: key)
  }

  public func saveValue(_ value: String, toKey key: String) {
    defaults.set(value, forKey: key)
  }

  public func removeKey(_ key: String) {
    defaults.removeObject(forKey: key)
  }
}

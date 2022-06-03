@_implementationOnly import EnvoyEngine
import Foundation

/// `KeyValueStore` is an interface that may be implemented to provide access to an arbitrary
/// key-value store implementation that may be made accessible to native Envoy Mobile code.

public protocol KeyValueStore {
  func readValue(forKey key: String) -> String?
  func saveValue(_ value: String, toKey key: String)
  func removeKey(_ key: String)
}

internal class KeyValueStoreImpl: EnvoyKeyValueStore {
  internal let implementation: KeyValueStore

  init(implementation: KeyValueStore) {
    self.implementation = implementation
  }

  func readValue(forKey key: String) -> String? {
    return implementation.readValue(forKey: key)
  }

  func saveValue(_ value: String, toKey key: String) {
    implementation.saveValue(value, toKey: key)
  }

  func removeKey(_ key: String) {
    implementation.removeKey(key)
  }
}

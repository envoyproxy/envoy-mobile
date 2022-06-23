import Foundation

private let kRestrictedPrefixes = [":", "x-envoy-mobile"]

private func isRestrictedHeader(name: String) -> Bool {
  return name.lowercased() == "host" || kRestrictedPrefixes.contains { name.lowercased().hasPrefix($0) }
}

/// Base builder class used to construct `Headers` instances.
/// See `{Request|Response}HeadersBuilder` for usage.
@objcMembers
public class HeadersBuilder: NSObject {

  struct CaseInsensitiveKey: Hashable {
    let name: String
    lazy var lowercasedName: String = { self.name.lowercased() }()

    static func == (lhs: CaseInsensitiveKey, rhs: CaseInsensitiveKey) -> Bool {
      var lhs = lhs, rhs = rhs
      return lhs.lowercasedName == rhs.lowercasedName
    }

    func hash(into hasher: inout Hasher) -> Int {
      hasher.combine(self.name.hashValue)
      return hasher.finalize()
    }
  }

  private(set) var _headers: [CaseInsensitiveKey: [String]]

  var headers: [String: [String]] {
    return Dictionary(uniqueKeysWithValues: _headers.map { key, value in
    var k = key
     return (k.lowercasedName, value) 
     })
  }

  /// Append a value to the header name.
  ///
  /// - parameter name:  The header name.
  /// - parameter value: The value associated to the header name.
  ///
  /// - returns: This builder.
  @discardableResult
  public func add(name: String, value: String) -> Self {
    if isRestrictedHeader(name: name) {
      return self
    }

    self._headers[CaseInsensitiveKey(name: name), default: []].append(value)
    return self
  }

  /// Replace all values at the provided name with a new set of header values.
  ///
  /// - parameter name: The header name.
  /// - parameter value: The value associated to the header name.
  ///
  /// - returns: This builder.
  @discardableResult
  public func set(name: String, value: [String]) -> Self {
    if isRestrictedHeader(name: name) {
      return self
    }

    self._headers[CaseInsensitiveKey(name: name)] = value
    return self
  }

  /// Remove all headers with this name.
  ///
  /// - parameter name: The header name to remove.
  ///
  /// - returns: This builder.
  @discardableResult
  public func remove(name: String) -> Self {
    if isRestrictedHeader(name: name) {
      return self
    }

    self._headers[CaseInsensitiveKey(name: name)] = nil
    return self
  }

  // MARK: - Internal

  /// Allows for setting headers that are not publicly mutable (i.e., restricted headers).
  ///
  /// - parameter name: The header name.
  /// - parameter value: The value associated to the header name.
  ///
  /// - returns: This builder.
  @discardableResult
  func internalSet(name: String, value: [String]) -> Self {
    self._headers[CaseInsensitiveKey(name: name)] = value
    return self
  }

  // Only explicitly implemented to work around a swiftinterface issue in Swift 5.1. This can be
  // removed once envoy is only built with Swift 5.2+
  public override init() {
    self._headers = [:]
    super.init()
  }

  /// Initialize a new builder. Subclasses should provide their own public convenience initializers.
  ///
  /// - parameter headers: The headers with which to start.
  required init(headers: [String: [String]]) {
    self._headers = Dictionary(uniqueKeysWithValues: headers.map { key, value in (CaseInsensitiveKey(name: key), value) })
    super.init()
  }
}

// MARK: - Equatable

extension HeadersBuilder {
  public override func isEqual(_ object: Any?) -> Bool {
    return (object as? Self)?.headers == self.headers
  }
}

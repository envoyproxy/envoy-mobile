import Foundation

private let kRestrictedPrefixes = [":", "x-envoy-mobile"]

private func isRestrictedHeader(name: String) -> Bool {
  return name == "host" || kRestrictedPrefixes.contains { name.hasPrefix($0) }
}

/// Base builder class used to construct `Headers` instances.
/// It preserves the original casing of headers and enforces a case
/// insensitive look up and setting of headers.
/// See `{Request|Response}HeadersBuilder` for usage.
@objcMembers
public class HeadersBuilder: NSObject {
  struct KeyValuesPair {
    private(set) var key: String
    private(set) var values: [String]

    init(key: String, values: [String] = []) {
      self.key = key
      self.values = values
    }

    mutating func appendValue(_ value: String) {
      self.values.append(value)
    }

    mutating func appendValues(_ values: [String]) {
      self.values.append(contentsOf: values)
    }

    mutating func setValue(_ value: String) {
      self.values = [value]
    }
  }

  private var _headers: [String: KeyValuesPair]

  func headers() -> [String: [String]] {
    return Dictionary(uniqueKeysWithValues: self._headers.map { _, value in
        return (value.key, value.values)
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
    let lowercasedName = name.lowercased()
    if isRestrictedHeader(name: lowercasedName) {
      return self
    }

    self._headers[lowercasedName, default: KeyValuesPair(key: name)].appendValue(value)
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
    let lowercasedName = name.lowercased()
    if isRestrictedHeader(name: lowercasedName) {
      return self
    }

    self._headers[lowercasedName] = KeyValuesPair(key: name, values: value)
    return self
  }

  /// Remove all headers with this name.
  ///
  /// - parameter name: The header name to remove.
  ///
  /// - returns: This builder.
  @discardableResult
  public func remove(name: String) -> Self {
    let lowercasedName = name.lowercased()
    if isRestrictedHeader(name: lowercasedName) {
      return self
    }

    self._headers[lowercasedName] = nil
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
    self._headers[name.lowercased()] = KeyValuesPair(key: name, values: value)
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
    var processedHeaders = [String: KeyValuesPair]()
    for (name, values) in headers {
      let lowercasedName = name.lowercased()
      /// Dictionaries in Swift are unordered collections. We process headers with keys
      /// that are the same when lowercased in an alphabetical order to avoid a situation
      /// in which the result of the initialization is underministic i.e., we want
      /// "[A: ["1"]", "a: ["2"]]" headers to be always converted to ["A": ["1", "2"]] and
      /// never to "a": ["2", "1"].
      ///
      /// If a given header name already exists in the processed headers map, check
      /// if the currently processed header name is before the existing header name as
      /// determined by an alphabetical order.
      if let existing = processedHeaders[lowercasedName], existing.key > name {
        processedHeaders[lowercasedName] =
          KeyValuesPair(key: name, values: values + existing.values)
      } else {
        processedHeaders[lowercasedName, default: KeyValuesPair(key: name)].appendValues(values)
      }
    }

    self._headers = processedHeaders
    super.init()
  }
}

// MARK: - Equatable

extension HeadersBuilder {
  public override func isEqual(_ object: Any?) -> Bool {
    return (object as? Self)?.headers() == self.headers()
  }
}

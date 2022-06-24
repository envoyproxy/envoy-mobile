import Foundation

/// Base class that is used to represent header/trailer data structures.
/// To instantiate new instances, see `{Request|Response}HeadersBuilder`.
@objcMembers
public class Headers: NSObject {
  let container: HeadersContainer

  /// Get the value for the provided header name.
  ///
  /// - parameter name: The case-insensitive header name for which to
  ///                   get the current value.
  ///                   
  ///
  /// - returns: The current headers specified for the provided name.
  public func value(forName name: String) -> [String]? {
    return self.container.value(forName: name)
  }

  /// Accessor for all underlying headers as a map.
  ///
  /// - returns: The underlying headers.
  public func allHeaders() -> [String: [String]] {
    return self.container.allHeaders()
  }

  /// Internal initializer used by builders.
  ///
  /// - parameter container: Headers to set.
  required init(container: HeadersContainer) {
    self.container = container
    super.init()
  }

  /// Inialize the receiver with a given headers map.
  ///
  /// - parameter headers: The headers map to use.
  convenience init(headers: [String: [String]]) {
    self.init(container: HeadersContainer(headers: headers))
  }

  override convenience init() {
    self.init(headers: [:])
  }
}

// MARK: - Equatable

extension Headers {
  public override func isEqual(_ object: Any?) -> Bool {
    return (object as? Self)?.container == self.container
  }
}

// MARK: - CustomStringConvertible

extension Headers {
  public override var description: String {
    return "\(type(of: self)) \(self.container.description)"
  }
}

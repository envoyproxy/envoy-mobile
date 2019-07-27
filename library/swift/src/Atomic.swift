import Foundation

/// <#function description#>
final class Atomic<T> {
  private let queue = DispatchQueue(label: "io.envoy-mobile.Atomic<\(T.self)>",
                                    qos: .default, attributes: .concurrent)
  private var storage: T

  /// <#function description#>
  var value: T {
    return self.queue.sync { self.storage }
  }

  /// <#function description#>
  ///
  /// - parameter value: <#value description#>
  init(_ value: T) {
    self.storage = value
  }

  /// <#function description#>
  ///
  /// - parameter closure: <#closure description#>
  func perform(_ closure: @escaping (inout T) -> Void) {
    self.queue.async(flags: .barrier) { closure(&self.storage) }
  }
}

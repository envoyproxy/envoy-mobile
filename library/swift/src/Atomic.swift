import Foundation

/// <#function description#>
final class Atomic<T> {
  private let queue = DispatchQueue(label: "io.envoy-mobile.Atomic<\(T.self)>",
                                    qos: .default, attributes: .concurrent)
  private var storage: T

  /// <#function description#>
  ///
  /// - parameter value: <#value description#>
  init(_ value: T) {
    self.storage = value
  }

  /// <#function description#>
  ///
  /// - parameter closure: <#closure description#>
  func async(_ closure: @escaping (inout T) -> Void) {
    self.queue.async(flags: .barrier) { closure(&self.storage) }
  }

  /// <#function description#>
  ///
  /// - parameter closure: <#closure description#>
  func sync(_ closure: (inout T) -> Void) {
    return self.queue.sync { closure(&self.storage) }
  }
}

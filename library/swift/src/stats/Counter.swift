import Foundation

/// A time series counter.
@objc
public protocol Counter: AnyObject {
  /// Increment the counter by the given count.
  /// - returns: A status indicating if the action was successful.
  func increment(count: Int) -> Int32
}

extension Counter {
  /// Increment the counter by 1.
  /// - returns: A status indicating if the action was successful.
  public func increment() -> Int32{
    self.increment(count: 1)
  }
}

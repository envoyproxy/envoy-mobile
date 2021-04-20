import Foundation

/// A time series counter.
@objc
public protocol Counter: AnyObject {
  /// Increment the counter by the given count.
  func increment(count: Int)

  /// Gets a counter with tags attached.
  func attach(tags: [Tag]) -> Counter
}

extension Counter {
  /// Increment the counter by 1.
  public func increment() {
    self.increment(count: 1)
  }
}

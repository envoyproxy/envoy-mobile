import Foundation

extension Data {
  /// Returns the next integer in the data, using the size of `T`. Nil if the data is too small.
  ///
  /// - returns: The next integer in the data, or nil.
  func nextInteger<T: FixedWidthInteger>() -> T? {
    let size = MemoryLayout<T>.size
    guard self.count >= size else {
      return nil
    }

    var value: T = 0
    _ = Swift.withUnsafeMutableBytes(of: &value) { valuePointer in
      self.copyBytes(to: valuePointer, count: size)
    }

    return value
  }
}

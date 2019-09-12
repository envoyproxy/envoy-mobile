import Foundation

extension Data {
  /// Gets the integer at the provided index using the size of `T`.
  /// Returned value includes the native endianness applied to it.
  /// Returns nil if the data is too small.
  ///
  /// - parameter index: The index at which to get the integer value.
  ///
  /// - returns: The next integer in the data, or nil.
  func integer<T: FixedWidthInteger>(atIndex index: Int) -> T? {
    let size = MemoryLayout<T>.size
    guard self.count >= index + size else {
      return nil
    }

    var value: T = 0
    _ = Swift.withUnsafeMutableBytes(of: &value) { valuePointer in
      self.copyBytes(to: valuePointer, from: index ..< index + size)
    }

    return value.withPlatformEndianness()
  }
}

extension FixedWidthInteger {
  /// Applies the native endianness of the current platform to the integer.
  ///
  /// - returns: The value, with big or little endianness applied.
  func withPlatformEndianness() -> Self {
    return self.deviceIsBigEndian() ? self.bigEndian : self.littleEndian
  }

  private func deviceIsBigEndian() -> Bool {
      let number: UInt32 = 0x12345678
      return number == number.bigEndian
  }
}

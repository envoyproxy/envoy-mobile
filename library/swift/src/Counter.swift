@_implementationOnly import EnvoyEngine
import Foundation

@objcMembers
public class Counter: NSObject {
  private let elements: [String]
  private weak var engine: EnvoyEngine?

  internal init(elements: [String], engine: EnvoyEngine) {
    self.elements = elements
    self.engine = engine
    super.init()
  }

  func increment(count: Int) {
    guard let engine = self.engine else {
      return
    }

    engine.recordCounter(self.elements.joined(separator: "."), count: numericCast(count))
  }
}

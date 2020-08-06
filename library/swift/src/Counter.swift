@_implementationOnly import EnvoyEngine
import Foundation

@objc
public class Counter: NSObject {
  private let series: String
  private weak var engine: EnvoyEngine?

  internal init(elements: [Element], engine: EnvoyEngine) {
    self.series = elements.map{ $0.description }.joined(separator: ".")
    self.engine = engine
    super.init()
  }

  func increment(count: Int) {
    guard let engine = self.engine else {
      return
    }

    engine.recordCounter(self.series, count: numericCast(count))
  }
}

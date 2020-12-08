import Envoy
import Foundation

/// Example of a simple HTTP filter that adds a response header.
struct DemoStringAccessor: EnvoyStringAccessor {
  func getString() -> String {
    return "DemoStringAccessor"
  }
}

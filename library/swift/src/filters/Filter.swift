@_implementationOnly import EnvoyEngine
import Foundation

/// Interface representing a filter. See `RequestFilter` and `ResponseFilter` for more details.
public protocol Filter {
  /// A unique name for a filter implementation. Needed for extension registration.
  var name: String { get }
}

extension EnvoyHTTPFilter {
  /// Initializer
  convenience init(filter: Filter) {
    self.init()
    self.name = filter.name

    if let requestFilter = filter as? RequestFilter {
      self.onRequestHeaders = {
        let result = requestFilter.onRequestHeaders(RequestHeaders(headers: $0), endStream: $1)
        switch result {
        case .continue(let headers):
          return [0, headers.headers]
        case .stopIteration(let headers):
          return [1, headers.headers]
        }
      }
    }

    if let responseFilter = filter as? ResponseFilter {
      self.onResponseHeaders = {
        let result = responseFilter.onResponseHeaders(ResponseHeaders(headers: $0), endStream: $1)
        switch result {
        case .continue(let headers):
          return [0, headers.headers]
        case .stopIteration(let headers):
          return [1, headers.headers]
        }
      }
    }
  }
}

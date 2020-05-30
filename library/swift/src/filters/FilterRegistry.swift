/// Contains a registry of filter factories and may be used for creating filter chains
/// to be used with outbound requests/streams.
final class FilterRegistry {
  private var factories = [() -> Filter]()

  /// Register a new filter factory that will be called to instantiate new filter instances for
  /// outbound requests/streams.
  ///
  /// - parameter factory: Closure that, when called, will return a new instance of a filter.
  ///                      The filter may be a `RequestFilter`, `ResponseFilter`, or both.
  func register(factory: @escaping () -> Filter) {
    self.factories.append(factory)
  }

  /// Create a set of filters from the registry.
  ///
  /// - returns: The set of filters (both request and response filters) from the registry,
  ///            listed in the order they were added.
  func createFilters() -> [Filter] {
    return self.factories.map { $0() }
  }
}

public final class FilterRegistry {
  private var factories = [() -> Filter]()

  public func register(factory: @escaping () -> Filter) {
    self.factories.append(factory)
  }

  func createChain() -> (requestChain: [RequestFilter], responseChain: [ResponseFilter]) {
    let filters = self.factories.map { $0() }
    return (filters.compactMap { $0 as? RequestFilter },
            filters.reversed().compactMap { $0 as? ResponseFilter })
  }
}

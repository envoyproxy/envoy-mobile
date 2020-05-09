final class FilterRegistry {
  private var factories = [() -> Filter]()

  func register(factory: @escaping () -> Filter) {
    self.factories.append(factory)
  }

  func createChain() -> (requestChain: [RequestFilter], responseChain: [ResponseFilter]) {
    let filters = self.factories.map { $0() }
    return (filters.compactMap { $0 as? RequestFilter },
            filters.compactMap { $0 as? ResponseFilter })
  }
}

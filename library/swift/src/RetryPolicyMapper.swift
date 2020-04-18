extension RetryPolicy {
  /// Converts the retry policy to a set of headers recognized by Envoy.
  ///
  /// - returns: The header representation of the retry policy.
  func outboundHeaders() -> [String: [String]] {
    var headers = [
      "x-envoy-max-retries": ["\(self.maxRetryCount)"],
      "x-envoy-retry-on": self.retryOn
        .lazy
        .map { $0.stringValue },
      "x-envoy-upstream-rq-timeout-ms": ["\(self.totalUpstreamTimeoutMS ?? 0)"],
    ]

    if let perRetryTimeoutMS = self.perRetryTimeoutMS {
      headers["x-envoy-upstream-rq-per-try-timeout-ms"] = ["\(perRetryTimeoutMS)"]
    }

    return headers
  }

  /// Initialize the retry policy from a set of headers.
  ///
  /// - parameter headers: The headers with which to initialize the retry policy.
  convenience init?(headers: Headers) {
    guard let maxRetryCount = headers.value(forName: "x-envoy-max-retries")?.first.flatMap(UInt.init) else {
      return nil
    }

    self.init(
      maxRetryCount: maxRetryCount,
      retryOn: headers.value(forName: "x-envoy-retry-on")?.compactMap(RetryRule.init) ?? [],
      perRetryTimeoutMS: headers.value(forName: "x-envoy-upstream-rq-per-try-timeout-ms")?
        .first.flatMap(UInt.init),
      totalUpstreamTimeoutMS: headers.value(forName: "x-envoy-upstream-rq-timeout-ms")?
        .first.flatMap(UInt.init)
    )
  }
}

extension RetryPolicy {
  /// Converts the retry policy to a set of headers recognized by Envoy.
  ///
  /// - returns: The header representation of the retry policy.
  func toHeaders() -> [String: String] {
    var headers = [
      "x-envoy-max-retries": "\(self.maxRetryCount)",
      "x-envoy-retry-on": self.retryOn
        .lazy
        .map { $0.stringValue }
        .joined(separator: ","),
    ]

    if let perRetryTimeoutMS = self.perRetryTimeoutMS {
      headers["x-envoy-upstream-rq-per-try-timeout-ms"] = "\(perRetryTimeoutMS)"
    }

    return headers
  }
}

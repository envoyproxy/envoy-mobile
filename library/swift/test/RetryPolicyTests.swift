@testable import Envoy
import XCTest

final class RetryPolicyTests: XCTestCase {
  // MARK: - Conversions

  func testConvertingToHeadersWithPerRetryTimeoutIncludesAllHeaders() {
    let policy = RetryPolicy(maxRetryCount: 123,
                             retryOn: RetryRule.allCases,
                             perRetryTimeoutMS: 9001)
    let expectedHeaders = [
      "x-envoy-max-retries": "123",
      "x-envoy-retry-on": "5xx,gateway-error,connect-failure,retriable-4xx,refused-upstream",
      "x-envoy-upstream-rq-per-try-timeout-ms": "9001",
    ]

    XCTAssertEqual(expectedHeaders, policy.toHeaders())
  }

  func testConvertingToHeadersWithoutRetryTimeoutExcludesPerRetryTimeoutHeader() {
    let policy = RetryPolicy(maxRetryCount: 123,
                             retryOn: RetryRule.allCases,
                             perRetryTimeoutMS: nil)
    let expectedHeaders = [
      "x-envoy-max-retries": "123",
      "x-envoy-retry-on": "5xx,gateway-error,connect-failure,retriable-4xx,refused-upstream",
    ]

    XCTAssertEqual(expectedHeaders, policy.toHeaders())
  }

  // MARK: - Equatability

  func testRetryPoliciesAreEqualWhenPropertiesAreEqual() {
    let policy1 = RetryPolicy(maxRetryCount: 123,
                              retryOn: [.connectFailure],
                              perRetryTimeoutMS: 8000)
    let policy2 = RetryPolicy(maxRetryCount: 123,
                              retryOn: [.connectFailure],
                              perRetryTimeoutMS: 8000)
    XCTAssertEqual(policy1, policy2)
  }

  func testPointersAreNotEqualWhenInstancesAreDifferent() {
    let policy1 = RetryPolicy(maxRetryCount: 123,
                              retryOn: [.connectFailure],
                              perRetryTimeoutMS: 8000)
    let policy2 = RetryPolicy(maxRetryCount: 123,
                              retryOn: [.connectFailure],
                              perRetryTimeoutMS: 8000)
    XCTAssert(policy1 !== policy2)
  }
}

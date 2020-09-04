import Envoy
import EnvoyEngine
import Foundation
import XCTest

private let kMockTemplate = """
mock_template:
- name: mock
  stats_domain: {{ stats_domain }}
  device_os: {{ device_os }}
  connect_timeout: {{ connect_timeout_seconds }}s
  dns_refresh_rate: {{ dns_refresh_rate_seconds }}s
  dns_failure_refresh_rate:
    base_interval: {{ dns_failure_refresh_rate_seconds_base }}s
    max_interval: {{ dns_failure_refresh_rate_seconds_max }}s
  platform_filter_chain: {{ platform_filter_chain }}
  stats_flush_interval: {{ stats_flush_interval_seconds }}s
  app_version: {{ app_version }}
  app_id: {{ app_id }}
  virtual_clusters: {{ virtual_clusters }}
"""

private struct TestFilter: Filter {}

final class BridgeTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
  }

  func testInitial() throws {
    // swiftlint:disable:next line_length
    let apiListenerType = "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager"
    // swiftlint:disable:next line_length
    let assertionFilterType = "type.googleapis.com/envoymobile.extensions.filters.http.assertion.Assertion"
    let config =
    """
    static_resources:
      listeners:
      - name: base_api_listener
        address:
          socket_address:
            protocol: TCP
            address: 0.0.0.0
            port_value: 10000
        api_listener:
          api_listener:
            "@type": \(apiListenerType)
            stat_prefix: hcm
            route_config:
              name: api_router
              virtual_hosts:
                - name: api
                  domains:
                    - "*"
                  routes:
                    - match:
                        prefix: "/"
                      direct_response:
                        status: 418
            http_filters:
              - name: envoy.filters.http.assertion
                typed_config:
                  "@type": \(assertionFilterType)
                  match_config:
                    http_request_headers_match:
                      headers:
                        - name: ":authority"
                          exact_match: example.com
              - name: envoy.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
    """
    let cond = NSCondition()
    let q = DispatchQueue(label: "test.envoymobile")
    let client = try EngineBuilder(yaml: config)
      .addLogLevel(.debug)
      .addFilter(factory: DemoFilter.init)
      .build()
      .streamClient

    let requestHeaders = RequestHeadersBuilder(method: .get, scheme: "https",
                                               authority: "example.com", path: "/test")
      .addUpstreamHttpProtocol(.http2)
      .build()
    client
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _ in
        let status = responseHeaders.httpStatus ?? -1
         XCTAssertEqual(200, status)
      }
      .setOnResponseData { _, endStream in
        XCTAssertTrue(endStream)
        cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: true)

    cond.wait()
  }
}

import Envoy
import EnvoyEngine
import Foundation
import XCTest

final class CancelStreamTests: XCTestCase {
  func testCancelStream() throws {
    // swiftlint:disable:next line_length
    let apiListenerType = "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager"
    // swiftlint:disable:next line_length
    let platformBridgeFilterType = "type.googleapis.com/envoymobile.extensions.filters.http.platform_bridge.PlatformBridge"
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
                        status: 200
            http_filters:
              - name: envoy.filters.http.platform_bridge
                typed_config:
                  "@type": \(platformBridgeFilterType)
                  platform_filter_name: cancel_validation_filter
              - name: envoy.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
    """

    struct CancelValidationFilter: ResponseFilter {
      let expectation: XCTestExpectation

      func onResponseHeaders(_ headers: ResponseHeaders, endStream: Bool)
        -> FilterHeadersStatus<ResponseHeaders>
      {
        return .continue(headers: headers)
      }

      func onResponseData(_ body: Data, endStream: Bool) -> FilterDataStatus<ResponseHeaders> {
        return .continue(data: body)
      }

      func onResponseTrailers(_ trailers: ResponseTrailers)
          -> FilterTrailersStatus<ResponseHeaders, ResponseTrailers> {
        return .continue(trailers: trailers)
      }

      func onError(_ error: EnvoyError) {}

      func onCancel() {
        self.expectation.fulfill()
      }
    }

    let runExpectation = self.expectation(description: "Run called with expected cancellation")
    let filterExpectation = self.expectation(description: "Filter called with cancellation")

    let client = try EngineBuilder(yaml: config)
      .addLogLevel(.debug)
      .addPlatformFilter(
        name: "cancel_validation_filter",
        factory: { CancelValidationFilter(expectation: filterExpectation) }
      )
      .build()
      .streamClient()

    client
      .newStreamPrototype()
      .setOnCancel {
         runExpectation.fulfill()
      }
      .start()
      .cancel()

    XCTAssertEqual(XCTWaiter.wait(for: [filterExpectation, runExpectation], timeout: 1), .completed)
  }
}

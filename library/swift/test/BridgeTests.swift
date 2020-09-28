import Envoy
import EnvoyEngine
import Foundation
import XCTest

// This test suite tests end-to-end integration of the platform layer to the core layer's HTTP
// functionality. It tests both the request side sendHeaders/Data,Close,Cancel; and the response
// side via all the setOnResponseHeaders/Data,setOnError,setOnCancel.
// TODO: setOnTrailers is not tested as the neither the direct_response pathway, nor the router
// allow sending trailers programatically. Add tests once possible.
final class HttpBridgeTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
  }

  func testSendHeaders() throws {
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
                        status: 200
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
      .streamClient()

    let requestHeaders = RequestHeadersBuilder(method: .get, scheme: "https",
                                               authority: "example.com", path: "/test")
      .addUpstreamHttpProtocol(.http2)
      .build()
    client
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _ in
        let status = responseHeaders.httpStatus ?? -1
         XCTAssertEqual(200, status)
         cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: true)

    cond.wait()
  }

  func testSendData() throws {
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
                        status: 200
            http_filters:
              - name: envoy.filters.http.assertion
                typed_config:
                  "@type": \(assertionFilterType)
                  match_config:
                    http_request_generic_body_match:
                      patterns:
                        - string_match: match_me
              - name: envoy.filters.http.buffer
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.buffer.v3.Buffer
                  max_request_bytes: 65000
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
      .streamClient()

    let requestHeaders = RequestHeadersBuilder(method: .get, scheme: "https",
                                               authority: "example.com", path: "/test")
      .addUpstreamHttpProtocol(.http2)
      .build()
    client
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _ in
        let status = responseHeaders.httpStatus ?? -1
         XCTAssertEqual(200, status)
         cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: false)
      .close(data: "match_me".data(using: .utf8)!)

    cond.wait()
  }

  func testSendTrailers() throws {
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
                        status: 200
            http_filters:
              - name: envoy.filters.http.assertion
                typed_config:
                  "@type": \(assertionFilterType)
                  match_config:
                    http_request_trailers_match:
                      headers:
                        - name: "test-trailer"
                          exact_match: test.code
              - name: envoy.filters.http.buffer
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.buffer.v3.Buffer
                  max_request_bytes: 65000
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
      .streamClient()

    let requestHeaders = RequestHeadersBuilder(method: .get, scheme: "https",
                                               authority: "example.com", path: "/test")
      .addUpstreamHttpProtocol(.http2)
      .build()
    let requestTrailers = RequestTrailersBuilder()
      .add(name: "test-trailer", value: "test.code")
      .build()

    client
      .newStreamPrototype()
      .setOnResponseHeaders { responseHeaders, _ in
        let status = responseHeaders.httpStatus ?? -1
         XCTAssertEqual(200, status)
         cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: false)
      .sendData("match_me".data(using: .utf8)!)
      .close(trailers: requestTrailers)

    cond.wait()
  }

  func testCancelStream() throws {
    // swiftlint:disable:next line_length
    let apiListenerType = "type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager"
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
      .streamClient()

    client
      .newStreamPrototype()
      .setOnCancel {
         cond.signal()
      }
      .start(queue: q)
      .cancel()

    cond.wait()
  }

  func testReceiveBody() throws {
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
                        status: 200
                        body:
                          inline_string: response_body
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
      .streamClient()

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
      .setOnResponseData { data, _ in
        let responseBody = String(decoding: data, as: UTF8.self)
        XCTAssertEqual("response_body", responseBody)
        cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: true)

    cond.wait()
  }

    func testReceiveError() throws {
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
                        status: 503
            http_filters:
              - name: envoy.filters.http.assertion
                typed_config:
                  "@type": \(assertionFilterType)
                  match_config:
                    http_request_generic_body_match:
                      patterns:
                        - string_match: match_me
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
      .streamClient()

    let requestHeaders = RequestHeadersBuilder(method: .get, scheme: "https",
                                               authority: "example.com", path: "/test")
      .addUpstreamHttpProtocol(.http2)
      .build()
    client
      .newStreamPrototype()
      .setOnError { _ in
         cond.signal()
      }
      .start(queue: q)
      .sendHeaders(requestHeaders, endStream: true)
    cond.wait()
  }

}

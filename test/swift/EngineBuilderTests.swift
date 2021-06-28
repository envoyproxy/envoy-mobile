@testable import Envoy
import EnvoyEngine
import Foundation
import XCTest

private let kMockTemplate =
"""
fixture_template:
- name: mock
  clusters:
#{custom_clusters}
  listeners:
#{custom_listeners}
  filters:
#{custom_filters}
  routes:
#{custom_routes}
"""

private struct TestFilter: Filter {}

final class EngineBuilderTests: XCTestCase {
  override func tearDown() {
    super.tearDown()
    MockEnvoyEngine.onRunWithConfig = nil
    MockEnvoyEngine.onRunWithTemplate = nil
  }

  func testCustomConfigTemplateUsesSpecifiedYAMLWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithTemplate = { yaml, _, _ in
      XCTAssertEqual("foobar", yaml)
      expectation.fulfill()
    }

    _ = EngineBuilder(yaml: "foobar")
      .addEngineType(MockEnvoyEngine.self)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingLogLevelAddsLogLevelWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { _, logLevel in
      XCTAssertEqual("trace", logLevel)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addLogLevel(.trace)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddinggrpcStatsDomainAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual("stats.envoyproxy.io", config.grpcStatsDomain)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addGrpcStatsDomain("stats.envoyproxy.io")
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingConnectTimeoutSecondsAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(12345, config.connectTimeoutSeconds)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addConnectTimeoutSeconds(12345)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingDNSRefreshSecondsAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(23, config.dnsRefreshSeconds)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addDNSRefreshSeconds(23)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingPlatformFiltersToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(1, config.httpPlatformFilterFactories.count)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addPlatformFilter(TestFilter.init)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingDNSFailureRefreshSecondsAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(1234, config.dnsFailureRefreshSecondsBase)
      XCTAssertEqual(5678, config.dnsFailureRefreshSecondsMax)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addDNSFailureRefreshSeconds(base: 1234, max: 5678)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingStatsFlushSecondsAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(42, config.statsFlushSeconds)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addStatsFlushSeconds(42)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingStreamIdleTimeoutSecondsAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(42, config.streamIdleTimeoutSeconds)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addStreamIdleTimeoutSeconds(42)
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingAppVersionAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual("v1.2.3", config.appVersion)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addAppVersion("v1.2.3")
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingAppIdAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual("com.envoymobile.ios", config.appId)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addAppId("com.envoymobile.ios")
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingVirtualClustersAddsToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual("[test]", config.virtualClusters)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addVirtualClusters("[test]")
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingNativeFiltersToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual(1, config.nativeFilterChain.count)
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addNativeFilter(name: "test_name", typedConfig: "config")
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testAddingStringAccessorToConfigurationWhenRunningEnvoy() {
    let expectation = self.expectation(description: "Run called with expected data")
    MockEnvoyEngine.onRunWithConfig = { config, _ in
      XCTAssertEqual("hello", config.stringAccessors["name"]?.getEnvoyString())
      expectation.fulfill()
    }

    _ = EngineBuilder()
      .addEngineType(MockEnvoyEngine.self)
      .addStringAccessor(name: "name", accessor: { "hello" })
      .build()
    self.waitForExpectations(timeout: 0.01)
  }

  func testResolvesYAMLWithIndividuallySetValues() throws {
    let config = EnvoyConfiguration(
      grpcStatsDomain: "stats.envoyproxy.io",
      connectTimeoutSeconds: 200,
      dnsRefreshSeconds: 300,
      dnsFailureRefreshSecondsBase: 400,
      dnsFailureRefreshSecondsMax: 500,
      statsFlushSeconds: 600,
      streamIdleTimeoutSeconds: 700,
      appVersion: "v1.2.3",
      appId: "com.envoymobile.ios",
      virtualClusters: "[test]",
      directResponseMatchers: "",
      directResponses: "",
      nativeFilterChain: [
        EnvoyNativeFilterConfig(name: "filter_name", typedConfig: "test_config"),
      ],
      platformFilterChain: [
        EnvoyHTTPFilterFactory(filterName: "TestFilter", factory: TestFilter.init),
      ],
      stringAccessors: [:]
    )
    let resolvedYAML = try XCTUnwrap(config.resolveTemplate(kMockTemplate))
    XCTAssertTrue(resolvedYAML.contains("&connect_timeout 200s"))
    XCTAssertTrue(resolvedYAML.contains("&dns_refresh_rate 300s"))
    XCTAssertTrue(resolvedYAML.contains("&dns_fail_base_interval 400s"))
    XCTAssertTrue(resolvedYAML.contains("&dns_fail_max_interval 500s"))
    XCTAssertTrue(resolvedYAML.contains("&stream_idle_timeout 700s"))

    // Metadata
    XCTAssertTrue(resolvedYAML.contains("device_os: iOS"))
    XCTAssertTrue(resolvedYAML.contains("app_version: v1.2.3"))
    XCTAssertTrue(resolvedYAML.contains("app_id: com.envoymobile.ios"))

    XCTAssertTrue(resolvedYAML.contains("&virtual_clusters [test]"))

    // Stats
    XCTAssertTrue(resolvedYAML.contains("&stats_domain stats.envoyproxy.io"))
    XCTAssertTrue(resolvedYAML.contains("&stats_flush_interval 600s"))

    // Filters
    XCTAssertTrue(resolvedYAML.contains("filter_name: TestFilter"))
    XCTAssertTrue(resolvedYAML.contains("name: filter_name"))
    XCTAssertTrue(resolvedYAML.contains("typed_config: test_config"))
  }

  func testReturnsNilWhenUnresolvedValueInTemplate() {
    let config = EnvoyConfiguration(
      grpcStatsDomain: "stats.envoyproxy.io",
      connectTimeoutSeconds: 200,
      dnsRefreshSeconds: 300,
      dnsFailureRefreshSecondsBase: 400,
      dnsFailureRefreshSecondsMax: 500,
      statsFlushSeconds: 600,
      streamIdleTimeoutSeconds: 700,
      appVersion: "v1.2.3",
      appId: "com.envoymobile.ios",
      virtualClusters: "[test]",
      directResponseMatchers: "",
      directResponses: "",
      nativeFilterChain: [],
      platformFilterChain: [],
      stringAccessors: [:]
    )
    XCTAssertNil(config.resolveTemplate("{{ missing }}"))
  }
}

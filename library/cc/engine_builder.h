#pragma once

#include <memory>
#include <string>

#include "engine.h"
#include "engine_callbacks.h"
#include "log_level.h"

namespace Envoy {
namespace Platform {

class EngineBuilder {
public:
  EngineBuilder(std::string config_template);
  EngineBuilder();

  EngineBuilder& addLogLevel(LogLevel log_level);
  EngineBuilder& setOnEngineRunning(std::function<void()> closure);

  EngineBuilder& addGrpcStatsDomain(const std::string& stats_domain);
  EngineBuilder& addConnectTimeoutSeconds(int connect_timeout_seconds);
  EngineBuilder& addDnsRefreshSeconds(int dns_refresh_seconds);
  EngineBuilder& addDnsFailureRefreshSeconds(int base, int max);
  EngineBuilder& addDnsQueryTimeoutSeconds(int dns_query_timeout_seconds);
  EngineBuilder& addDnsPreresolveHostnames(const std::string& dns_preresolve_hostnames);
  EngineBuilder& addH2ConnectionKeepaliveIdleIntervalMilliseconds(
      int h2_connection_keepalive_idle_interval_milliseconds);
  EngineBuilder&
  addH2ConnectionKeepaliveTimeoutSeconds(int h2_connection_keepalive_timeout_seconds);
  EngineBuilder& addH2StreamBufferLimitBytes(int h2_stream_buffer_limit_bytes);
  EngineBuilder& addStatsFlushSeconds(int stats_flush_seconds);
  EngineBuilder& addVirtualClusters(const std::string& virtual_clusters);
  EngineBuilder& setAppVersion(const std::string& app_version);
  EngineBuilder& setAppId(const std::string& app_id);
  EngineBuilder& setDeviceOs(const std::string& app_id);

  // this is separated from build() for the sake of testability
  std::string generateConfigStr();

  EngineSharedPtr build();

  // TODO(crockeo): add after filter integration
  // EngineBuilder& addPlatformFilter(name: String = UUID.randomUUID().toString(), factory: () ->
  // Filter): EngineBuilder& addNativeFilter(name: String = UUID.randomUUID().toString(),
  // typedConfig: String): EngineBuilder& addStringAccessor(name: String, accessor:
  // EnvoyStringAccessor): EngineBuilder {

private:
  LogLevel log_level_ = LogLevel::info;
  EngineCallbacksSharedPtr callbacks_;

  std::string config_template_;
  std::string stats_domain_ = "0.0.0.0";
  int connect_timeout_seconds_ = 30;
  int dns_refresh_seconds_ = 60;
  int dns_failure_refresh_seconds_base_ = 2;
  int dns_failure_refresh_seconds_max_ = 10;
  int dns_query_timeout_seconds_ = 25;
  std::string dns_preresolve_hostnames_ = "[]";
  int h2_connection_keepalive_idle_interval_milliseconds_ = 100000000;
  int h2_connection_keepalive_timeout_seconds_ = 10;
  // 10Mb default (10 * 1024 * 1024)
  int h2_stream_buffer_limit_bytes_ = 10485760;
  int stats_flush_seconds_ = 60;
  std::string app_version_ = "unspecified";
  std::string app_id_ = "unspecified";
  std::string device_os_ = "unspecified";
  std::string virtual_clusters_ = "[]";
  int stream_idle_timeout_seconds_ = 15;
  int per_try_idle_timeout_seconds_ = 15;

  // TODO(crockeo): add after filter integration
  // private var platformFilterChain = mutableListOf<EnvoyHTTPFilterFactory>()
  // private var nativeFilterChain = mutableListOf<EnvoyNativeFilterConfig>()
  // private var stringAccessors = mutableMapOf<String, EnvoyStringAccessor>()
};

using EngineBuilderSharedPtr = std::shared_ptr<EngineBuilder>;

} // namespace Platform
} // namespace Envoy

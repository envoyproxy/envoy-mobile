#include "engine_builder.h"

#include <sstream>

#include "fmt/core.h"
#include "library/common/main_interface.h"

namespace Envoy {
namespace Platform {

EngineBuilder::EngineBuilder(std::string config_template) : config_template_(config_template) {}
EngineBuilder::EngineBuilder() : EngineBuilder(std::string(config_template)) {}

EngineBuilder& EngineBuilder::addLogLevel(LogLevel log_level) {
  log_level_ = log_level;
  callbacks_ = std::make_shared<EngineCallbacks>();
  return *this;
}

EngineBuilder& EngineBuilder::setOnEngineRunning(std::function<void()> closure) {
  callbacks_->on_engine_running = closure;
  return *this;
}

EngineBuilder& EngineBuilder::addGrpcStatsDomain(const std::string& stats_domain) {
  stats_domain_ = stats_domain;
  return *this;
}

EngineBuilder& EngineBuilder::addConnectTimeoutSeconds(int connect_timeout_seconds) {
  connect_timeout_seconds_ = connect_timeout_seconds;
  return *this;
}

EngineBuilder& EngineBuilder::addDnsRefreshSeconds(int dns_refresh_seconds) {
  dns_refresh_seconds_ = dns_refresh_seconds;
  return *this;
}

EngineBuilder& EngineBuilder::addDnsFailureRefreshSeconds(int base, int max) {
  dns_failure_refresh_seconds_base_ = base;
  dns_failure_refresh_seconds_max_ = max;
  return *this;
}

EngineBuilder& EngineBuilder::addDnsQueryTimeoutSeconds(int dns_query_timeout_seconds) {
  dns_query_timeout_seconds_ = dns_query_timeout_seconds;
  return *this;
}

EngineBuilder&
EngineBuilder::addDnsPreresolveHostnames(const std::string& dns_preresolve_hostnames) {
  dns_preresolve_hostnames_ = dns_preresolve_hostnames;
  return *this;
}

EngineBuilder& EngineBuilder::addH2ConnectionKeepaliveIdleIntervalMilliseconds(
    int h2_connection_keepalive_idle_interval_milliseconds) {
  h2_connection_keepalive_idle_interval_milliseconds_ =
      h2_connection_keepalive_idle_interval_milliseconds;
  return *this;
}

EngineBuilder&
EngineBuilder::addH2ConnectionKeepaliveTimeoutSeconds(int h2_connection_keepalive_timeout_seconds) {
  h2_connection_keepalive_timeout_seconds_ = h2_connection_keepalive_timeout_seconds;
  return *this;
}

EngineBuilder& EngineBuilder::addStatsFlushSeconds(int stats_flush_seconds) {
  stats_flush_seconds_ = stats_flush_seconds;
  return *this;
}

EngineBuilder& EngineBuilder::addVirtualClusters(const std::string& virtual_clusters) {
  virtual_clusters_ = virtual_clusters;
  return *this;
}

EngineBuilder& EngineBuilder::setAppVersion(const std::string& app_version) {
  app_version_ = app_version;
  return *this;
}

EngineBuilder& EngineBuilder::setAppId(const std::string& app_id) {
  app_id_ = app_id;
  return *this;
}

EngineBuilder& EngineBuilder::setDeviceOs(const std::string& device_os) {
  device_os_ = device_os;
  return *this;
}

std::string EngineBuilder::generateConfigStr() {
  std::vector<std::pair<std::string, std::string>> replacements{
      {"connect_timeout", fmt::format("{}s", connect_timeout_seconds_)},
      {"dns_fail_base_interval", fmt::format("{}s", dns_failure_refresh_seconds_base_)},
      {"dns_fail_max_interval", fmt::format("{}s", dns_failure_refresh_seconds_max_)},
      {"dns_preresolve_hostnames", dns_preresolve_hostnames_},
      {"dns_refresh_rate", fmt::format("{}s", dns_refresh_seconds_)},
      {"dns_query_timeout", fmt::format("{}s", dns_query_timeout_seconds_)},
      {"h2_connection_keepalive_idle_interval",
       fmt::format("{}s", h2_connection_keepalive_idle_interval_milliseconds_ / 1000.0)},
      {"h2_connection_keepalive_timeout",
       fmt::format("{}s", h2_connection_keepalive_timeout_seconds_)},
      {
          "metadata",
          fmt::format("{{ device_os: {}, app_version: {}, app_id: {} }}", device_os_, app_version_,
                      app_id_),
      },
      {"stats_domain", stats_domain_},
      {"stats_flush_interval", fmt::format("{}s", stats_flush_seconds_)},
      {"stream_idle_timeout", fmt::format("{}s", stream_idle_timeout_seconds_)},
      {"per_try_idle_timeout", fmt::format("{}s", per_try_idle_timeout_seconds_)},
      {"virtual_clusters", virtual_clusters_},
  };

  // NOTE: this does not include support for custom filters
  // which are not yet supported in the C++ platform implementation
  std::ostringstream config_builder;
  config_builder << "!ignore platform_defs:" << std::endl;
  for (const auto& [key, value] : replacements) {
    config_builder << "- &" << key << " " << value << std::endl;
  }
  config_builder << config_template_;

  auto config_str = config_builder.str();
  if (config_str.find("{{") != std::string::npos) {
    throw std::runtime_error("could not resolve all template keys in config");
  }
  return config_str;
}

EngineSharedPtr EngineBuilder::build() {
  envoy_logger null_logger;
  null_logger.log = nullptr;
  null_logger.release = envoy_noop_const_release;
  null_logger.context = nullptr;

  envoy_event_tracker null_tracker{};

  auto config_str = generateConfigStr();
  auto envoy_engine = init_engine(callbacks_->asEnvoyEngineCallbacks(), null_logger, null_tracker);
  run_engine(envoy_engine, config_str.c_str(), logLevelToString(log_level_).c_str());

  // we can't construct via std::make_shared
  // because Engine is only constructible as a friend
  Engine* engine = new Engine(envoy_engine);
  auto engine_ptr = EngineSharedPtr(engine);
  return engine_ptr;
}

} // namespace Platform
} // namespace Envoy

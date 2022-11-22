#include "test/common/integration/xds_integration_test.h"
#include "xds_integration_test.h"

#include "envoy/config/bootstrap/v3/bootstrap.pb.h"
#include "envoy/config/cluster/v3/cluster.pb.h"

#include "test/common/grpc/grpc_client_integration.h"
#include "test/common/integration/base_client_integration_test.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"
#include "library/common/config/internal.h"

#include "gtest/gtest.h"

namespace Envoy {

using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;

XdsIntegrationTest::XdsIntegrationTest() : BaseClientIntegrationTest(ipVersion()) {
  override_builder_config_ = false;
  expect_dns_ = false;              // TODO(alyssawilk) debug.
  create_xds_upstream_ = true;
  sotw_or_delta_ = sotwOrDelta();

  if (sotw_or_delta_ == Grpc::SotwOrDelta::UnifiedSotw ||
      sotw_or_delta_ == Grpc::SotwOrDelta::UnifiedDelta) {
    config_helper_.addRuntimeOverride("envoy.reloadable_features.unified_mux", "true");
  }

  // Write the engine builder config to the bootstrap.  
  // This ensures the fake upstreams match what's been specified in the engine
  config_helper_.addConfigModifier([this](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
    auto config_str = this->generateConfigStr();
    TestUtility::loadFromYaml(absl::StrCat(config_header, config_str), bootstrap);
  });
  enableCustomClusters(true);
  this->setIpvVersion(Network::Test::getLoopbackAddressString(ipVersion()));
  disableStatsConfig(true);

  // xDS upstream is created separately in the test infra, and there's only one non-xDS cluster.
  setUpstreamCount(1);
  setAdminConfig(adminConfig());
  admin_filename_ = TestEnvironment::temporaryPath("admin_address.txt");
  setAdminAddressPathForTests(admin_filename_);
}

void XdsIntegrationTest::initialize() {
  BaseClientIntegrationTest::initialize();
  default_request_headers_.setScheme("https");
}

Network::Address::IpVersion XdsIntegrationTest::ipVersion() const {
  return std::get<0>(GetParam());
}

Grpc::ClientType XdsIntegrationTest::clientType() const { return std::get<1>(GetParam()); }

Grpc::SotwOrDelta XdsIntegrationTest::sotwOrDelta() const { return std::get<2>(GetParam()); }

void XdsIntegrationTest::SetUp() {
  // TODO(abeyad): Add paramaterized tests for HTTP1, HTTP2, and HTTP3.
  setUpstreamProtocol(Http::CodecType::HTTP2);
}

void XdsIntegrationTest::TearDown() {
  cleanup();
  BaseClientIntegrationTest::TearDown();
}

void XdsIntegrationTest::createEnvoy() {
  BaseClientIntegrationTest::createEnvoy();
  std::string admin_str = TestEnvironment::readFileToStringForTest(admin_filename_);
  auto addr = Network::Utility::parseInternetAddressAndPort(admin_str);
  registerPort("admin", addr->ip()->port());
  if (on_server_init_function_) {
    on_server_init_function_();
  }
}

void XdsIntegrationTest::initializeXdsStream() {
  createXdsConnection();
  AssertionResult result =
      xds_connection_->waitForNewStream(*BaseIntegrationTest::dispatcher_, xds_stream_);
  RELEASE_ASSERT(result, result.message());
  xds_stream_->startGrpcStream();
}

std::string XdsIntegrationTest::getRuntimeKey(const std::string& key) {
  auto response = IntegrationUtil::makeSingleRequest(
      lookupPort("admin"), "GET", "/runtime?format=json", "", Http::CodecType::HTTP2, version_);
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().getStatusValue());
  Json::ObjectSharedPtr loader = TestEnvironment::jsonLoadFromString(response->body());
  auto entries = loader->getObject("entries");
  if (entries->hasObject(key)) {
    return entries->getObject(key)->getString("final_value");
  }
  return "";
}

// TODO(abeyad): Change this implementation to use the PulseClient once implemented. See
// https://github.com/envoyproxy/envoy-mobile/issues/2356 for details.
uint64_t XdsIntegrationTest::getCounterValue(const std::string& counter) {
  auto response = IntegrationUtil::makeSingleRequest(lookupPort("admin"), "GET", "/stats?usedonly",
                                                     "", Http::CodecType::HTTP2, version_);
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().getStatusValue());
  std::stringstream ss(response->body());
  std::string line;
  while (std::getline(ss, line, '\n')) {
    auto pos = line.find(':');
    if (pos == std::string::npos) {
      continue;
    }
    if (line.substr(0, pos) == counter) {
      return std::stoi(line.substr(pos + 1));
    }
  }
  return 0;
}

AssertionResult XdsIntegrationTest::waitForCounterGe(const std::string& name, uint64_t value) {
  constexpr std::chrono::milliseconds timeout = TestUtility::DefaultTimeout;
  Event::TestTimeSystem::RealTimeBound bound(timeout);
  while (getCounterValue(name) < value) {
    Event::GlobalTimeSystem().timeSystem().advanceTimeWait(std::chrono::milliseconds(10));
    if (timeout != std::chrono::milliseconds::zero() && !bound.withinBound()) {
      return AssertionFailure() << fmt::format("timed out waiting for {} to be {}", name, value);
    }
  }
  return AssertionSuccess();
}

envoy::config::cluster::v3::Cluster
XdsIntegrationTest::createSingleEndpointClusterConfig(const std::string& cluster_name) {
  envoy::config::cluster::v3::Cluster config;
  config.set_name(cluster_name);

  // Set the endpoint.
  auto* load_assignment = config.mutable_load_assignment();
  load_assignment->set_cluster_name(cluster_name);
  auto* endpoint = load_assignment->add_endpoints()->add_lb_endpoints()->mutable_endpoint();
  endpoint->mutable_address()->mutable_socket_address()->set_address(
      Network::Test::getLoopbackAddressString(ipVersion()));
  endpoint->mutable_address()->mutable_socket_address()->set_port_value(0);

  // Set the protocol options.
  envoy::extensions::upstreams::http::v3::HttpProtocolOptions options;
  options.mutable_explicit_http_config()->mutable_http2_protocol_options();
  (*config.mutable_typed_extension_protocol_options())
      ["envoy.extensions.upstreams.http.v3.HttpProtocolOptions"]
          .PackFrom(options);
  return config;
}

std::string XdsIntegrationTest::adminConfig() {
  const std::string yaml = fmt::format(R"EOF(
admin:
  address:
    socket_address:
      address: {}
      port_value: 0
)EOF",
                                       Network::Test::getLoopbackAddressString(ipVersion()));

  return yaml;
}

} // namespace Envoy

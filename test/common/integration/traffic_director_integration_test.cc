#include "envoy/admin/v3/config_dump.pb.h"
#include "envoy/config/bootstrap/v3/bootstrap.pb.h"
#include "envoy/config/cluster/v3/cluster.pb.h"
#include "envoy/config/core/v3/base.pb.h"
#include "envoy/grpc/status.h"

#include "source/common/protobuf/protobuf.h"
#include "source/common/protobuf/utility.h"
#include "source/common/version/version.h"

#include "test/integration/http_integration.h"
#include "test/integration/utility.h"
#include "test/test_common/network_utility.h"
#include "test/test_common/resources.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

using testing::AllOf;
using testing::AssertionResult;
using testing::Contains;

namespace Envoy {

/*************************  A T T E N T I O N  *************************/

// First, make sure you have a GCP project and it is set up to use Traffic Director, following the
// instructions in https://cloud.google.com/traffic-director/docs/set-up-gce-vms.

// Next, create a private key for your project if you don't already have one:
//   1. Go to "Service Accounts" for your project in Google Cloud Console.
//   2. Click on the "Keys" tab.
//   3. Click on "Add Key" -> "Create New Key".
//   4. For the "Key type", choose "JSON".
//   5. Click "Create".
//   6. A file will be downloaded that contains your private key and other metadata.

// Lastly, change the variables below to your GCP project's values and credentials.

// The name of your project in Google Cloud Console; e.g. my-td-testing-project.
constexpr char PROJECT_NAME[] = "<CHANGE ME>";
// The Project Number of your project, found on the main page of the project in Google Cloud
// Console, e.g. 798832730858.
constexpr char PROJECT_ID[] = "<CHANGE ME>";
// Copy from the "client_id" field in the private key JSON file you generated.
constexpr char CLIENT_ID[] = "<CHANGE ME>";
// Copy from the "private_key_id" field in the private key JSON file you generated.
constexpr char PRIVATE_KEY_ID[] = "<CHANGE ME>";
// Copy from the "private_key" field in the private key JSON file you generated.
constexpr char PRIVATE_KEY[] = "<CHANGE ME>";

/*************************  END VARIABLES SECTION  *************************/

std::string jwtToken() {
  const std::string email = fmt::format("{}-compute@developer.gserviceaccount.com", PROJECT_ID);
  const std::string cert_url = fmt::format("https://www.googleapis.com/robot/v1/metadata/x509/"
                                           "{}-compute%40developer.gserviceaccount.com",
                                           PROJECT_ID);
  return fmt::format(R"EOF({{
  "type": "service_account",
  "project_id": "{}",
  "private_key_id": "{}",
  "private_key": "{}",
  "client_email": "{}",
  "client_id": "{}",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "{}"
}})EOF",
                     PROJECT_NAME, PRIVATE_KEY_ID, PRIVATE_KEY, email, CLIENT_ID, cert_url);
}

void setJwtToken(envoy::config::core::v3::ApiConfigSource& api_config_source) {
  api_config_source.mutable_grpc_services(0)
      ->mutable_google_grpc()
      ->mutable_call_credentials(0)
      ->mutable_service_account_jwt_access()
      ->set_json_key(jwtToken());
}

std::string bootstrapConfig() {
  const std::string bootstrap_yaml = R"EOF(
node:
  id: "projects/798832730858/networks/default/nodes/111222333444"
  cluster: cluster  # unused
  locality:
    zone: "us-central1-a"
dynamic_resources:
  lds_config:
    ads: {}
    resource_api_version: V3
  cds_config:
    ads: {}
    resource_api_version: V3
  ads_config:
    api_type: GRPC
    transport_api_version: V3
    grpc_services:
    - google_grpc:
        target_uri: trafficdirector.googleapis.com:443
        stat_prefix: trafficdirector
        channel_credentials:
          ssl_credentials:
            root_certs:
              filename: /etc/ssl/certs/ca-certificates.crt
        channel_args:
          args:
            grpc.http2.max_pings_without_data:
              int_value: 0
            grpc.keepalive_time_ms:
              int_value: 10000
            grpc.keepalive_timeout_ms:
              int_value: 20000
        call_credentials:
          service_account_jwt_access:
            token_lifetime_seconds: 31536000
            json_key: ""
cluster_manager:
  load_stats_config:
    api_type: GRPC
    transport_api_version: V3
    grpc_services:
    - google_grpc:
        target_uri: trafficdirector.googleapis.com:443
        stat_prefix: trafficdirector
        channel_credentials:
          ssl_credentials:
            root_certs:
              filename: /etc/ssl/certs/ca-certificates.crt
        channel_args:
          args:
            grpc.http2.max_pings_without_data:
              int_value: 0
            grpc.keepalive_time_ms:
              int_value: 10000
            grpc.keepalive_timeout_ms:
              int_value: 20000
        call_credentials:
          service_account_jwt_access:
            token_lifetime_seconds: 31536000
            json_key: ""
admin:
  access_log_path: /dev/stdout
  address:
    socket_address:
      address: 127.0.0.1  # Admin page is only accessible locally.
      port_value: 15000
  )EOF";

  envoy::config::bootstrap::v3::Bootstrap bootstrap;
  TestUtility::loadFromYaml(bootstrap_yaml, bootstrap);

  setJwtToken(*bootstrap.mutable_dynamic_resources()->mutable_ads_config());
  setJwtToken(*bootstrap.mutable_cluster_manager()->mutable_load_stats_config());

  return MessageUtil::getYamlStringFromMessage(bootstrap);
}

class TrafficDirectorIntegrationTest : public HttpIntegrationTest,
                                       public testing::TestWithParam<Network::Address::IpVersion> {
public:
  TrafficDirectorIntegrationTest()
      : HttpIntegrationTest(Http::CodecType::HTTP2, ipVersion(), bootstrapConfig()) {
    // Forces the test server's Envoy to use the node metadata in the bootstrap config.
    use_bootstrap_node_metadata_ = true;

    // Other test knobs.
    skip_tag_extraction_rule_check_ = true;
    use_lds_ = false;
    create_xds_upstream_ = false;
    tls_xds_upstream_ = false;
  }

  Network::Address::IpVersion ipVersion() const { return GetParam(); }

  envoy::admin::v3::ClustersConfigDump getClustersConfigDump() {
    auto message_ptr = test_server_->server().admin().getConfigTracker().getCallbacksMap().at(
        "clusters")(Matchers::UniversalStringMatcher());
    return dynamic_cast<const envoy::admin::v3::ClustersConfigDump&>(*message_ptr);
  }

  std::vector<std::string>
  getDynamicActiveClusterNames(const envoy::admin::v3::ClustersConfigDump& config_dump) {
    std::vector<std::string> resource_names;
    resource_names.reserve(config_dump.dynamic_active_clusters().size());
    for (const auto& dynamic_cluster : config_dump.dynamic_active_clusters()) {
      envoy::config::cluster::v3::Cluster cluster;
      dynamic_cluster.cluster().UnpackTo(&cluster);
      resource_names.push_back(cluster.name());
    }
    return resource_names;
  }
};

INSTANTIATE_TEST_SUITE_P(IpVersions, TrafficDirectorIntegrationTest,
                         testing::ValuesIn(TestEnvironment::getIpVersionsForTest()));

TEST_P(TrafficDirectorIntegrationTest, Basic) {
  // Starts up Envoy and loads the bootstrap config, which will trigger the dynamic cluster
  // configuration to be fetched from Traffic Director.
  initialize();

  // Wait for the xDS cluster resources to be retrieved and loaded.
  test_server_->waitForGaugeGe("cluster_manager.active_clusters", 3);

  // Verify the dynamic cluster resources from Traffic Director are what we expect.
  EXPECT_THAT(getDynamicActiveClusterNames(getClustersConfigDump()),
              AllOf(Contains("cloud-internal-istio:cloud_mp_798832730858_1578897841695688881"),
                    Contains("cloud-internal-istio:cloud_mp_798832730858_4497773746904456309"),
                    Contains("cloud-internal-istio:cloud_mp_798832730858_523871542841416155")));
}

} // namespace Envoy

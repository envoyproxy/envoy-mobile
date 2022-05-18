#include "source/extensions/http/header_formatters/preserve_case/config.h"
#include "source/extensions/http/header_formatters/preserve_case/preserve_case_formatter.h"

#include "test/common/http/common.h"
#include "test/integration/autonomous_upstream.h"
#include "test/integration/integration.h"
#include "test/server/utility.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/cc/engine_builder.h"
#include "library/common/config/internal.h"
#include "library/common/data/utility.h"
#include "library/common/http/client.h"
#include "library/common/http/header_utility.h"
#include "library/common/types/c_types.h"

using testing::ReturnRef;

namespace Envoy {
namespace {

// Based on Http::Utility::toRequestHeaders() but only used for these tests.
Http::ResponseHeaderMapPtr toResponseHeaders(envoy_headers headers) {
  std::unique_ptr<Http::ResponseHeaderMapImpl> transformed_headers =
      Http::ResponseHeaderMapImpl::create();
  transformed_headers->setFormatter(
      std::make_unique<
          Extensions::Http::HeaderFormatters::PreserveCase::PreserveCaseHeaderFormatter>(
          false, envoy::extensions::http::header_formatters::preserve_case::v3::
                     PreserveCaseFormatterConfig::DEFAULT));
  Http::Utility::toEnvoyHeaders(*transformed_headers, headers);
  return transformed_headers;
}

typedef struct {
  uint32_t on_headers_calls;
  uint32_t on_data_calls;
  uint32_t on_complete_calls;
  uint32_t on_error_calls;
  uint32_t on_cancel_calls;
  uint64_t on_header_consumed_bytes_from_response;
  uint64_t on_complete_received_byte_count;
  std::string status;
  ConditionalInitializer* terminal_callback;
} callbacks_called;

void validateStreamIntel(const envoy_final_stream_intel& final_intel) {
  EXPECT_NE(-1, final_intel.dns_start_ms);
  EXPECT_NE(-1, final_intel.dns_end_ms);
  // This test doesn't do TLS.
  EXPECT_EQ(-1, final_intel.ssl_start_ms);
  EXPECT_EQ(-1, final_intel.ssl_end_ms);

  ASSERT_NE(-1, final_intel.stream_start_ms);
  ASSERT_NE(-1, final_intel.connect_start_ms);
  ASSERT_NE(-1, final_intel.connect_end_ms);
  ASSERT_NE(-1, final_intel.sending_start_ms);
  ASSERT_NE(-1, final_intel.sending_end_ms);
  ASSERT_NE(-1, final_intel.response_start_ms);
  ASSERT_NE(-1, final_intel.stream_end_ms);

  ASSERT_LE(final_intel.stream_start_ms, final_intel.connect_start_ms);
  ASSERT_LE(final_intel.connect_start_ms, final_intel.connect_end_ms);
  ASSERT_LE(final_intel.connect_end_ms, final_intel.sending_start_ms);
  ASSERT_LE(final_intel.sending_start_ms, final_intel.sending_end_ms);
  ASSERT_LE(final_intel.response_start_ms, final_intel.stream_end_ms);
}

// TODO(junr03): move this to derive from the ApiListenerIntegrationTest after moving that class
// into a test lib.
class ClientIntegrationTest : public BaseIntegrationTest,
                              public testing::TestWithParam<Network::Address::IpVersion> {
public:
  ClientIntegrationTest() : BaseIntegrationTest(GetParam(), defaultConfig()) {
    use_lds_ = false;
    autonomous_upstream_ = true;
    defer_listener_finalization_ = true;
    HttpTestUtility::addDefaultHeaders(default_request_headers_);
    config_helper_.addConfigModifier([](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
      // The default stats config has overenthusiastic filters.
      bootstrap.clear_stats_config();
    });
    // TODO(alyssawilk) upstream has an issue with logical DNS ipv6 clusters -
    // remove this once #21359 lands.
    config_helper_.addConfigModifier([](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
      auto* static_resources = bootstrap.mutable_static_resources();
      for (int i = 0; i < static_resources->clusters_size(); ++i) {
        auto* cluster = static_resources->mutable_clusters(i);
        if (cluster->type() == envoy::config::cluster::v3::Cluster::LOGICAL_DNS) {
          cluster->clear_type();
        }
      }
    });
  }

  void initialize() override {
    BaseIntegrationTest::initialize();
    ConditionalInitializer server_started;
    test_server_->server().dispatcher().post([this, &server_started]() -> void {
      http_client_ = std::make_unique<Http::Client>(
          test_server_->server().listenerManager().apiListener()->get().http()->get(), *dispatcher_,
          test_server_->statStore(), test_server_->server().api().randomGenerator());
      dispatcher_->drain(test_server_->server().dispatcher());
      server_started.setReady();
    });
    server_started.waitReady();
    default_request_headers_.setHost(fake_upstreams_[0]->localAddress()->asStringView());
  }

  void SetUp() override {
    bridge_callbacks_.context = &cc_;
    bridge_callbacks_.on_headers = [](envoy_headers c_headers, bool, envoy_stream_intel intel,
                                      void* context) -> void* {
      Http::ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
      callbacks_called* cc_ = static_cast<callbacks_called*>(context);
      cc_->on_headers_calls++;
      cc_->status = response_headers->Status()->value().getStringView();
      cc_->on_header_consumed_bytes_from_response = intel.consumed_bytes_from_response;
      return nullptr;
    };
    bridge_callbacks_.on_data = [](envoy_data c_data, bool, envoy_stream_intel,
                                   void* context) -> void* {
      callbacks_called* cc_ = static_cast<callbacks_called*>(context);
      cc_->on_data_calls++;
      release_envoy_data(c_data);
      return nullptr;
    };
    bridge_callbacks_.on_complete = [](envoy_stream_intel, envoy_final_stream_intel final_intel,
                                       void* context) -> void* {
      validateStreamIntel(final_intel);
      callbacks_called* cc_ = static_cast<callbacks_called*>(context);
      cc_->on_complete_received_byte_count = final_intel.received_byte_count;
      cc_->on_complete_calls++;
      cc_->terminal_callback->setReady();
      return nullptr;
    };
    bridge_callbacks_.on_error = [](envoy_error error, envoy_stream_intel, envoy_final_stream_intel,
                                    void* context) -> void* {
      release_envoy_error(error);
      callbacks_called* cc_ = static_cast<callbacks_called*>(context);
      cc_->on_error_calls++;
      cc_->terminal_callback->setReady();
      return nullptr;
    };
    bridge_callbacks_.on_cancel = [](envoy_stream_intel, envoy_final_stream_intel,
                                     void* context) -> void* {
      callbacks_called* cc_ = static_cast<callbacks_called*>(context);
      cc_->on_cancel_calls++;
      cc_->terminal_callback->setReady();
      return nullptr;
    };
  }

  void TearDown() override {
    test_server_.reset();
    fake_upstreams_.clear();
  }

  static std::string bootstrap_config() {
    // At least one empty filter chain needs to be specified.
    return ConfigHelper::baseConfig() + R"EOF(
    filter_chains:
      filters:
    )EOF";
  }

  // Use the Envoy mobile default config as much as possible in this test.
  // There are some config modifiers below which do result in deltas.
  static std::string defaultConfig() {
    Platform::EngineBuilder builder;

    ProtobufMessage::StrictValidationVisitorImpl visitor;
    envoy::config::bootstrap::v3::Bootstrap bootstrap;
    std::string config_str = absl::StrCat(config_header, builder.generateConfigStr());
    return config_str;
  }

  Event::ProvisionalDispatcherPtr dispatcher_ = std::make_unique<Event::ProvisionalDispatcher>();
  Http::ClientPtr http_client_{};
  envoy_http_callbacks bridge_callbacks_;
  ConditionalInitializer terminal_callback_;
  callbacks_called cc_ = {0, 0, 0, 0, 0, 0, 0, "", &terminal_callback_};
  Http::TestRequestHeaderMapImpl default_request_headers_;
  envoy_stream_t stream_ = 1;
};

INSTANTIATE_TEST_SUITE_P(IpVersions, ClientIntegrationTest,
                         testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                         TestUtility::ipTestParamsToString);

TEST_P(ClientIntegrationTest, Basic) {
  initialize();

  bridge_callbacks_.on_data = [](envoy_data c_data, bool end_stream, envoy_stream_intel,
                                 void* context) -> void* {
    if (end_stream) {
      EXPECT_EQ(Data::Utility::copyToString(c_data), "");
    } else {
      EXPECT_EQ(c_data.length, 10);
    }
    callbacks_called* cc_ = static_cast<callbacks_called*>(context);
    cc_->on_data_calls++;
    release_envoy_data(c_data);
    return nullptr;
  };

  // Build a set of request headers.
  Buffer::OwnedImpl request_data = Buffer::OwnedImpl("request body");
  default_request_headers_.addCopy(AutonomousStream::EXPECT_REQUEST_SIZE_BYTES,
                                   std::to_string(request_data.length()));

  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Build body data
  envoy_data c_data = Data::Utility::toBridgeData(request_data);

  // Build a set of request trailers.
  // TODO: update the autonomous upstream to assert on trailers, or to send trailers back.
  Http::TestRequestTrailerMapImpl trailers;
  envoy_headers c_trailers = Http::Utility::toBridgeHeaders(trailers);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, false);
    http_client_->sendData(stream_, c_data, false);
    http_client_->sendTrailers(stream_, c_trailers);
  });
  terminal_callback_.waitReady();

  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.status, "200");
  ASSERT_EQ(cc_.on_data_calls, 2);
  ASSERT_EQ(cc_.on_complete_calls, 1);
  ASSERT_EQ(cc_.on_header_consumed_bytes_from_response, 27);
  ASSERT_EQ(cc_.on_complete_received_byte_count, 67);

  // stream_success gets charged for 2xx status codes.
  test_server_->waitForCounterEq("http.client.stream_success", 1);
}

TEST_P(ClientIntegrationTest, BasicNon2xx) {
  initialize();

  // Set response header status to be non-2xx to test that the correct stats get charged.
  reinterpret_cast<AutonomousUpstream*>(fake_upstreams_.front().get())
      ->setResponseHeaders(std::make_unique<Http::TestResponseHeaderMapImpl>(
          Http::TestResponseHeaderMapImpl({{":status", "503"}, {"content-length", "0"}})));

  // Build a set of request headers.
  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, true);
  });
  terminal_callback_.waitReady();

  ASSERT_EQ(cc_.on_error_calls, 0);
  ASSERT_EQ(cc_.status, "503");
  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.on_complete_calls, 1);

  // stream_failure gets charged for all non-2xx status codes.
  test_server_->waitForCounterEq("http.client.stream_failure", 1);
}

TEST_P(ClientIntegrationTest, BasicReset) {
  initialize();

  // Cause an upstream reset after request is complete.
  default_request_headers_.addCopy(AutonomousStream::RESET_AFTER_REQUEST, "yes");
  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, true);
  });
  terminal_callback_.waitReady();

  ASSERT_EQ(cc_.on_error_calls, 1);
  ASSERT_EQ(cc_.on_headers_calls, 0);
  // Reset causes a charge to stream_failure.
  test_server_->waitForCounterEq("http.client.stream_failure", 1);
}

TEST_P(ClientIntegrationTest, BasicCancel) {
  autonomous_upstream_ = false;
  initialize();

  bridge_callbacks_.on_headers = [](envoy_headers c_headers, bool, envoy_stream_intel,
                                    void* context) -> void* {
    Http::ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    callbacks_called* cc_ = static_cast<callbacks_called*>(context);
    cc_->on_headers_calls++;
    cc_->status = response_headers->Status()->value().getStringView();
    // Lie and say the request is complete, so the test has something to wait
    // on.
    cc_->terminal_callback->setReady();
    return nullptr;
  };

  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, true);
  });

  Envoy::FakeRawConnectionPtr upstream_connection;
  ASSERT_TRUE(fake_upstreams_[0]->waitForRawConnection(upstream_connection));

  std::string upstream_request;
  EXPECT_TRUE(upstream_connection->waitForData(FakeRawConnection::waitForInexactMatch("GET /"),
                                               &upstream_request));

  // Send an incomplete response.
  auto response = "HTTP/1.1 200 OK\r\nContent-Length: 15\r\n\r\n";
  ASSERT_TRUE(upstream_connection->write(response));
  // For this test only, the terminal callback is called when headers arrive.
  terminal_callback_.waitReady();
  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.status, "200");
  ASSERT_EQ(cc_.on_data_calls, 0);
  ASSERT_EQ(cc_.on_complete_calls, 0);

  // Now cancel, and make sure the cancel is received.
  dispatcher_->post([&]() -> void { http_client_->cancelStream(stream_); });
  terminal_callback_.waitReady();
  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.status, "200");
  ASSERT_EQ(cc_.on_data_calls, 0);
  ASSERT_EQ(cc_.on_complete_calls, 0);
  ASSERT_EQ(cc_.on_cancel_calls, 1);
}

// TODO(junr03): test with envoy local reply with local stream not closed, which causes a reset
// fired from the Http:ConnectionManager rather than the Http::Client. This cannot be done in
// unit tests because the Http::ConnectionManager is mocked using a mock response encoder.

// Test header key case sensitivity.
TEST_P(ClientIntegrationTest, CaseSensitive) {
  Envoy::Extensions::Http::HeaderFormatters::PreserveCase::
      forceRegisterPreserveCaseFormatterFactoryConfig();
  config_helper_.addConfigModifier([](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
    ConfigHelper::HttpProtocolOptions protocol_options;
    auto typed_extension_config = protocol_options.mutable_explicit_http_config()
                                      ->mutable_http_protocol_options()
                                      ->mutable_header_key_format()
                                      ->mutable_stateful_formatter();
    typed_extension_config->set_name("preserve_case");
    typed_extension_config->mutable_typed_config()->set_type_url(
        "type.googleapis.com/"
        "envoy.extensions.http.header_formatters.preserve_case.v3.PreserveCaseFormatterConfig");
    ConfigHelper::setProtocolOptions(*bootstrap.mutable_static_resources()->mutable_clusters(0),
                                     protocol_options);
  });

  autonomous_upstream_ = false;
  initialize();

  bridge_callbacks_.on_headers = [](envoy_headers c_headers, bool, envoy_stream_intel,
                                    void* context) -> void* {
    Http::ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    callbacks_called* cc_ = static_cast<callbacks_called*>(context);
    cc_->on_headers_calls++;
    cc_->status = response_headers->Status()->value().getStringView();
    EXPECT_EQ("My-ResponsE-Header",
              response_headers->formatter().value().get().format("my-response-header"));
    return nullptr;
  };

  // Build a set of request headers.
  default_request_headers_.header_map_->setFormatter(
      std::make_unique<
          Extensions::Http::HeaderFormatters::PreserveCase::PreserveCaseHeaderFormatter>(
          false, envoy::extensions::http::header_formatters::preserve_case::v3::
                     PreserveCaseFormatterConfig::DEFAULT));
  default_request_headers_.addCopy("FoO", "bar");
  default_request_headers_.header_map_->formatter().value().get().processKey("FoO");
  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, true);
  });

  Envoy::FakeRawConnectionPtr upstream_connection;
  ASSERT_TRUE(fake_upstreams_[0]->waitForRawConnection(upstream_connection));

  // Verify that the upstream request has preserved cased headers.
  std::string upstream_request;
  EXPECT_TRUE(upstream_connection->waitForData(FakeRawConnection::waitForInexactMatch("GET /"),
                                               &upstream_request));
  EXPECT_TRUE(absl::StrContains(upstream_request, "FoO: bar")) << upstream_request;

  // Verify that the downstream response has preserved cased headers.
  auto response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nMy-ResponsE-Header: foo\r\n\r\n";
  ASSERT_TRUE(upstream_connection->write(response));

  terminal_callback_.waitReady();

  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.status, "200");
  ASSERT_EQ(cc_.on_data_calls, 0);
  ASSERT_EQ(cc_.on_complete_calls, 1);

  // stream_success gets charged for 2xx status codes.
  test_server_->waitForCounterEq("http.client.stream_success", 1);
}

TEST_P(ClientIntegrationTest, Timeout) {
  config_helper_.addConfigModifier([](envoy::config::bootstrap::v3::Bootstrap& bootstrap) {
    auto* listener = bootstrap.mutable_static_resources()->mutable_listeners(0);
    auto* em_hcm = listener->mutable_api_listener()->mutable_api_listener();
    auto hcm =
        MessageUtil::anyConvert<envoy::extensions::filters::network::http_connection_manager::v3::
                                    EnvoyMobileHttpConnectionManager>(*em_hcm);
    hcm.mutable_config()->mutable_stream_idle_timeout()->set_seconds(1);
    em_hcm->PackFrom(hcm);
  });

  autonomous_upstream_ = false;
  initialize();

  bridge_callbacks_.on_headers = [](envoy_headers c_headers, bool, envoy_stream_intel,
                                    void* context) -> void* {
    Http::ResponseHeaderMapPtr response_headers = toResponseHeaders(c_headers);
    callbacks_called* cc_ = static_cast<callbacks_called*>(context);
    cc_->on_headers_calls++;
    cc_->status = response_headers->Status()->value().getStringView();
    return nullptr;
  };

  // Build a set of request headers.
  envoy_headers c_headers = Http::Utility::toBridgeHeaders(default_request_headers_);

  // Create a stream.
  dispatcher_->post([&]() -> void {
    http_client_->startStream(stream_, bridge_callbacks_, false);
    http_client_->sendHeaders(stream_, c_headers, false);
  });

  Envoy::FakeRawConnectionPtr upstream_connection;
  ASSERT_TRUE(fake_upstreams_[0]->waitForRawConnection(upstream_connection));

  std::string upstream_request;
  EXPECT_TRUE(upstream_connection->waitForData(FakeRawConnection::waitForInexactMatch("GET /"),
                                               &upstream_request));

  // Send response headers but no body.
  auto response = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\nMy-ResponsE-Header: foo\r\n\r\n";
  ASSERT_TRUE(upstream_connection->write(response));

  terminal_callback_.waitReady();

  ASSERT_EQ(cc_.on_headers_calls, 1);
  ASSERT_EQ(cc_.status, "200");
  ASSERT_EQ(cc_.on_data_calls, 0);
  ASSERT_EQ(cc_.on_complete_calls, 0);
  ASSERT_EQ(cc_.on_error_calls, 1);
}

} // namespace
} // namespace Envoy

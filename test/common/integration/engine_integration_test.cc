#include "test/integration/integration.h"

#include "gtest/gtest.h"
#include "library/common/bridge/utility.h"
#include "library/common/main_interface.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace {

class EngineIntegrationTest : public testing::Test {
public:
  EngineIntegrationTest()
      : api_(Envoy::Api::createApiForTest(stats_, time_)),
        dispatcher_(api_->allocateDispatcher("test_thread")) {
    callbacks_.context = this;
    callbacks_.on_error =
        +[](envoy_error error, envoy_stream_intel, envoy_final_stream_intel, void*) -> void* {
      release_envoy_error(error);
      // We don't expect to see errors with the current flows, update if we need more complex tests.
      EXPECT_TRUE(false);
      return nullptr;
    },
    callbacks_.on_cancel =
        +[](envoy_stream_intel, envoy_final_stream_intel, void* context) -> void* {
      EngineIntegrationTest* this_ptr = static_cast<EngineIntegrationTest*>(context);
      this_ptr->cancel_called_.Notify();
      return nullptr;
    };
    Envoy::FakeUpstreamConfig config(time_);
    config.upstream_protocol_ = Envoy::Http::CodecType::HTTP1;

    fake_upstream_ =
        std::make_unique<Envoy::FakeUpstream>(0, Envoy::Network::Address::IpVersion::v4, config);
  }

  void startEngine() {
    envoy_engine_callbacks callbacks{[](void* context) -> void {
                                       auto* this_ptr =
                                           static_cast<EngineIntegrationTest*>(context);
                                       this_ptr->engine_running_.Notify();
                                     },
                                     [](void* context) -> void {
                                       auto* this_ptr =
                                           static_cast<EngineIntegrationTest*>(context);
                                       this_ptr->engine_exit_.Notify();
                                     },
                                     this};

    engine_ = init_engine(callbacks, {}, {});
    envoy_status_t rc = run_engine(engine_, config_template, "debug");
    ASSERT_EQ(rc, ENVOY_SUCCESS);

    engine_running_.WaitForNotification();
  }

  void shutdownEngine() {
    terminate_engine(engine_);
    engine_exit_.WaitForNotification();
  }

  envoy_engine_t engine_;
  absl::Notification engine_running_;
  absl::Notification engine_exit_;
  envoy_http_callbacks callbacks_;
  absl::Notification cancel_called_;
  Envoy::FakeUpstreamPtr fake_upstream_;
  Envoy::FakeHttpConnectionPtr fake_connection_;

  Envoy::Stats::IsolatedStoreImpl stats_;
  Envoy::Event::TestRealTimeSystem time_;
  Envoy::Api::ApiPtr api_;
  Envoy::Event::DispatcherPtr dispatcher_;
};

TEST_F(EngineIntegrationTest, ShutdownCancelsActiveStreams) {
  startEngine();

  envoy_stream_t stream = init_stream(engine_);
  {
    envoy_status_t rc = start_stream(stream, callbacks_, false);
    ASSERT_EQ(rc, ENVOY_SUCCESS);
  }

  envoy_status_t rc =
      send_headers(stream,
                   Bridge::Utility::makeEnvoyMap(
                       {{":authority",
                         fmt::format("127.0.0.1:{}", fake_upstream_->localAddress()->ip()->port())},
                        {":scheme", "http"},
                        {":method", "GET"},
                        {":path", "/"}}),
                   false);
  ASSERT_EQ(rc, ENVOY_SUCCESS);

  ASSERT_TRUE(fake_upstream_->waitForHttpConnection(*dispatcher_, fake_connection_));

  shutdownEngine();

  cancel_called_.WaitForNotification();
}
} // namespace
} // namespace Envoy

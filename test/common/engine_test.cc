#include "absl/synchronization/notification.h"
#include "gtest/gtest.h"
#include "library/common/engine.h"
#include "library/common/main_interface.h"

namespace Envoy {

class EngineTest : public testing::Test {};

TEST_F(EngineTest, EarlyExit) {
  const std::string config = "admin: {}";
  const std::string level = "debug";
  absl::Notification done;
  envoy_engine_callbacks cbs{[](void* context) -> void {
                               auto* done = static_cast<absl::Notification*>(context);
                               done->Notify();
                             },
                             &done};

  run_engine(0, cbs, config.c_str(), level.c_str());

  terminate_engine(0);

  ASSERT_TRUE(done.WaitForNotificationWithTimeout(absl::Seconds(1)));

  start_stream(0, {});
}
} // namespace Envoy

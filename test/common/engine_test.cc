#include "common/buffer/buffer_impl.h"

#include "gtest/gtest.h"
#include "library/common/engine.h"
#include "library/common/main_interface.h"

namespace Envoy {

class EngineTest : public testing::Test {};

TEST_F(EngineTest, EarlyExit) {
  const std::string config = "";
  const std::string level = "debug";
  envoy_engine_callbacks cbs{[]() -> void {ASSERT_TRUE(false);}};

  run_engine(0, cbs, config.c_str(), level.c_str());

  // stop_loop();
}

} // namespace Envoy

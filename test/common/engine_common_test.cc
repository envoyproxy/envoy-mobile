#include "gtest/gtest.h"
#include "library/common/engine_common.h"

namespace Envoy {

TEST(EngineCommonTest, SignalHandlingFalse) {
  const std::string config_yaml = R"EOF(
layered_runtime:
  layers:
  - name: static_layer_0
    static_layer:
      overload:
        global_downstream_max_connections: 50000
)EOF";

  OptionsImpl options("", "", "", spdlog::level::level_enum::info);
  options.setConfigYaml(config_yaml);
  EngineCommon main_common(std::move(options));
  ASSERT_FALSE(main_common.server()->options().signalHandlingEnabled());
}

} // namespace Envoy

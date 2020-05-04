#include "gtest/gtest.h"
#include "library/common/envoy_mobile_main_common.h"

namespace Envoy {

TEST(MobileMainCommonTest, SignalHandlingFalse) {
  char* envoy_argv[] = {strdup("envoy"), strdup("--config-yaml"), strdup("{}"), nullptr};
  MobileMainCommon main_common{3, envoy_argv};
  ASSERT_FALSE(main_common.server()->options().signalHandlingEnabled());
}

} // namespace Envoy

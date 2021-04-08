#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/common/lambda_logger_delegate.h"

using testing::_;
using testing::HasSubstr;

namespace Envoy {
namespace Logger {

TEST(LambdaDelegate, LogCb) {
  std::string expected_msg = "Hello LambdaDelegate";
  std::string actual_msg;

  LambdaDelegate delegate = LambdaDelegate(
      [&actual_msg](absl::string_view msg) -> void { actual_msg = msg; }, Registry::getSink());

  ENVOY_LOG_MISC(error, expected_msg);
  EXPECT_THAT(actual_msg, HasSubstr(expected_msg));
}

} // namespace Logger
} // namespace Envoy

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

  LambdaDelegate delegate =
      LambdaDelegate([&actual_msg](absl::string_view msg) -> void { actual_msg = msg; },
                     []() -> void {}, Registry::getSink());

  ENVOY_LOG_MISC(error, expected_msg);
  EXPECT_THAT(actual_msg, HasSubstr(expected_msg));
}

TEST(LambdaDelegate, FlushCb) {
  uint i{0};
  LambdaDelegate delegate = LambdaDelegate([](absl::string_view) -> void {},
                                           [&i]() -> void { i++; }, Registry::getSink());

  EXPECT_EQ(i, 0U);
  Registry::getSink()->flush();
  EXPECT_EQ(i, 1U);
}

} // namespace Logger
} // namespace Envoy

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library/common/common/lambda_logger_delegate.h"

using testing::_;
using testing::HasSubstr;

namespace Envoy {
namespace Logger {

TEST(LambdaDelegate, LogCb) {
  uint i{0};
  std::string expected_msg = "Hello LambdaDelegate";

  LambdaDelegate delegate = LambdaDelegate(
      [&i, &expected_msg](absl::string_view msg) -> void {
        EXPECT_THAT(msg, HasSubstr(expected_msg));
        i++;
      },
      []() -> void {}, Registry::getSink());

  EXPECT_EQ(i, 0U);
  ENVOY_LOG_MISC(error, expected_msg);
  EXPECT_EQ(i, 1U);
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

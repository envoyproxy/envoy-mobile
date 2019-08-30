#include "common/buffer/buffer_impl.h"

#include "gtest/gtest.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Buffer {

TEST(DataConstructorTest, FromCToCppEmpty) {
  envoy_data empty_data = {0, nullptr, free, nullptr};

  InstancePtr cpp_data = Utility::toInternalData(empty_data);

  ASSERT_EQ(cpp_data->length(), 0);
}

TEST(DataConstructorTest, FromCToCpp) {
  std::string s = "test string";
  envoy_data c_data = {static_cast<int>(s.size()), reinterpret_cast<const uint8_t*>(s.c_str()),
                       free, nullptr};

  InstancePtr cpp_data = Utility::toInternalData(c_data);

  ASSERT_EQ(cpp_data->length(), c_data.length);
  ASSERT_EQ(cpp_data->toString(), s);
}

TEST(DataConstructorTest, FromCppToCEmpty) {
  OwnedImpl empty_data;

  absl::optional<envoy_data> maybe_c_data = Utility::toBridgeData(empty_data);

  ASSERT_TRUE(maybe_c_data.has_value());
  ASSERT_EQ(maybe_c_data.value().length, 0);
}

TEST(DataConstructorTest, FromCppToC) {
  std::string s = "test string";
  OwnedImpl cpp_data = OwnedImpl(absl::string_view(s));

  absl::optional<envoy_data> maybe_c_data = Utility::toBridgeData(cpp_data);
  ASSERT_TRUE(maybe_c_data.has_value());
  envoy_data c_data = maybe_c_data.value();

  ASSERT_EQ(c_data.length, s.size());
  ASSERT_EQ(Http::Utility::convertToString(c_data), s);
}

} // namespace Buffer
} // namespace Envoy

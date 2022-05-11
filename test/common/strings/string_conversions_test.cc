#include <algorithm>
#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "library/common/strings/string_conversions.h"

namespace Envoy {
namespace Strings {

namespace {

TEST(StringConversions, UTF8ToUTF16) {
  std::string utf8 = u8"zß水🍌";
  std::u16string utf16 = u"zß水🍌";

  // Some sanity checking before proceeding, the strings should:
  // - Have different code unit sizes when UTF-8 encoded vs UTF-16.
  // - Refer to the same abstract unicode characters.
  ASSERT_NE(utf8.size(), utf16.size());
  ASSERT_TRUE(std::lexicographical_compare(utf8.begin(), utf8.end(), utf16.begin(), utf16.end()));

  EXPECT_EQ(utf16, UTF8ToUTF16(utf8.data(), utf8.size()));
}

TEST(StringConversions, UTF16ToUTF8) {
  std::string utf8 = u8"zß水🍌";
  std::u16string utf16 = u"zß水🍌";

  // Some sanity checking before proceeding, the strings should:
  // - Have different code unit sizes when UTF-8 encoded vs UTF-16.
  // - Refer to the same abstract unicode characters.
  ASSERT_NE(utf8.size(), utf16.size());
  ASSERT_TRUE(std::lexicographical_compare(utf8.begin(), utf8.end(), utf16.begin(), utf16.end()));

  EXPECT_EQ(utf8, UTF16ToUTF8(utf16.data(), utf16.size()));
}

} // namespace
} // namespace Strings
} // namespace Envoy

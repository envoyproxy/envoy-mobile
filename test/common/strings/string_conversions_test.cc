#include <string>

#include "gtest/gtest.h"
#include "library/common/strings/string_conversions.h"

TEST(StringConversions, SameEncoding) {
  std::string utf8 = u8"z\u00df\u6c34\U0001f34c";
  std::u16string utf16 = u"z\u00df\u6c34\U0001f34c";

  EXPECT_EQ(utf16, UTF8ToUTF16(utf8.data(), utf8.size()));
}

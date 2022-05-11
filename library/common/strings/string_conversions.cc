#include "library/common/strings/string_conversions.h"

#include <codecvt>
#include <locale>

namespace Envoy {
namespace Strings {

std::string UTF16ToUTF8(const char16_t* utf16, size_t len) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(utf16,
                                                                                      utf16 + len);
}

std::u16string UTF8ToUTF16(const char* utf8, size_t len) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8,
                                                                                        utf8 + len);
}

} // namespace Strings
} // namespace Envoy

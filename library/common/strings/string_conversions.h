#pragma once

#include <string>

namespace Envoy {
namespace Strings {

/**
 * Converts a UTF-16 string to an UTF-8 one.
 * @param utf16, the array of code units composing the UTF-16 string
 * @param len, the number of code units
 * @return the input string encoded in UTF-8
 */
std::string UTF16ToUTF8(const char16_t* utf16, size_t len);

/**
 * Converts a UTF-8 string to an UTF-16 one.
 * @param utf8, the array of code units composing the UTF-8 string
 * @param len, the number of code units
 * @return the input string encoded in UTF-16
 */
std::u16string UTF8ToUTF16(const char* utf8, size_t len);

} // namespace Strings
} // namespace Envoy

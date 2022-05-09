#pragma once

#include <string>

std::string UTF16ToUTF8(const char16_t* utf16, size_t len);

std::u16string UTF8ToUTF16(const char* utf8, size_t len);

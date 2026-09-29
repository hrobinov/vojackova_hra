#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <string_view>

namespace agui
{
  namespace StringUtil
  {
    inline std::string boolToString(bool value) { return value ? "true" : "false"; }
    inline bool startsWith(std::string_view text, std::string_view prefix) { return text.substr(0, prefix.size()) == prefix; }
  }

  /** printf into a std::string. */
  inline std::string ssprintf(const char* format, ...)
  {
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    const int length = std::vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    std::string result(length > 0 ? size_t(length) : 0, '\0');
    if (length > 0)
      std::vsnprintf(result.data(), result.size() + 1, format, args);
    va_end(args);
    return result;
  }
}

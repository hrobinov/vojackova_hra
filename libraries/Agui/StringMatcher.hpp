#pragma once
#include "Agui/LowercaseString.hpp"
#include <string_view>

namespace agui
{
  enum class StringMatcherResult { NoMatch, Match };

  namespace StringMatcher
  {
    /** Case-insensitive substring match; filter is already lowercase. */
    inline StringMatcherResult matchesSearchPattern(std::string_view text, const LowercaseString& filter)
    {
      return LowercaseString(std::string(text)).str().find(filter.str()) != std::string::npos
        ? StringMatcherResult::Match : StringMatcherResult::NoMatch;
    }
  }
}

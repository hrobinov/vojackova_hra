#pragma once
#include <string>

namespace agui
{
  struct ImeCompositionInfo
  {
    std::string text;
    // The offset, in unicode codepoints, of the cursor, relative to the start of the text.
    int cursorOffset = 0;
  };
}

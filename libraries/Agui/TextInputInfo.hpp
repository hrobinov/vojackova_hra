#pragma once
#include <Agui/Point.hpp>
#include <Agui/Widget/TextBox.hpp>

namespace agui
{
  struct TextInputInfo
  {
    // Used primarily for the Nintendo Switch keyboard.
    GenericTargeter<TextBox> textBox;
    bool enabled = false;
    // The position, in screen coordinates, of the input area.
    Point pos;
    int width = 100;
    int height = 10;
    // The offset, in screen coordinates, of the cursor, relative to the left edge of the input area.
    int cursorOffset = 0;

    bool isNumeric = false;
    bool numericAllowDecimal = false;
    bool numericAllowNegative = false;
    bool isPassword = false;
    bool isMultiline = true;
  };
}

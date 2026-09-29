#pragma once
#include <cstdint>

namespace agui
{
  enum class MouseButton : uint16_t
  {
    NONE = 1 << 0,
    LEFT = 1 << 1,
    RIGHT = 1 << 2,
    MIDDLE = 1 << 3,
    BUTTON_4 = 1 << 4,
    BUTTON_5 = 1 << 5,
    BUTTON_6 = 1 << 6,
    BUTTON_7 = 1 << 7,
    BUTTON_8 = 1 << 8,
    BUTTON_9 = 1 << 9,
    LEFT_AND_RIGHT = LEFT | RIGHT,
    ALL = LEFT_AND_RIGHT | MIDDLE | BUTTON_4 | BUTTON_5 | BUTTON_6 | BUTTON_7 | BUTTON_8 | BUTTON_9
  };

  inline bool operator&(MouseButton a, MouseButton b)
  {
    return (uint16_t(a) & uint16_t(b)) != 0;
  }

  inline MouseButton operator|(MouseButton a, MouseButton b)
  {
    return agui::MouseButton(uint16_t(a) | uint16_t(b));
  }
}

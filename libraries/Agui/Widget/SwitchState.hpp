#pragma once
#include <cstdint>

namespace agui
{
  enum class SwitchState : uint8_t
  {
    // Saved and loaded so the values shouldn't change
    Left = 0,
    Right = 1,
    None = 2
  };
}

#pragma once
#include <cstdint>

namespace agui
{
  #undef Always
  enum class GameControllerInteraction : uint8_t
  {
    //Game controller will always hover this widget regardless of type or state, including when disabled
    Always = 0,
    //Hover according to the widget type and implementation
    Normal = 1,
    //Never hover this widget with a game controller
    Never = 2
  };
}

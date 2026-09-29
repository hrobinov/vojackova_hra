#pragma once
#include <string>
#include <stdint.h>

namespace agui
{
  enum class AreaAlign : uint8_t
  {
    LeftTop,
    LeftMiddle,
    LeftBottom,
    CenterTop,
    CenterMiddle,
    CenterBottom,
    RightTop,
    RightMiddle,
    RightBottom
  };
  AreaAlign parseAreaAlignmentFromString(std::string_view input);
  const char* areaAlignemntEnumToString(AreaAlign value);
}

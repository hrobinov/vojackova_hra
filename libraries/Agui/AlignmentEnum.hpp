#pragma once
#include "Agui/AreaAlignmentEnum.hpp"

namespace agui
{
  enum class HorizontalAlign : uint8_t
  {
    Left,
    Center,
    Right
  };

  enum class VerticalAlign : uint8_t
  {
    Top,
    Center,
    Bottom
  };

  HorizontalAlign toHorizontalAlign(AreaAlign areaAlign);
  VerticalAlign toVerticalAlign(AreaAlign areaAlign);
  AreaAlign combine(HorizontalAlign horizontalAlign, VerticalAlign verticalAlign);

  HorizontalAlign parseHorizontalAlignFromString(std::string_view input);
  const char* horizontalAlignToString(HorizontalAlign value);

  VerticalAlign parseVertialAlignFromString(std::string_view input);
  const char* verticalAlignToString(VerticalAlign value);
}

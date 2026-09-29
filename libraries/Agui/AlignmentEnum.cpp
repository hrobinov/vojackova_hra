#include "Agui/AlignmentEnum.hpp"
#include <stdexcept>

namespace agui
{
  HorizontalAlign toHorizontalAlign(AreaAlign areaAlign)
  {
    switch (areaAlign)
    {
      case AreaAlign::LeftTop:
      case AreaAlign::LeftMiddle:
      case AreaAlign::LeftBottom: return HorizontalAlign::Left;
      case AreaAlign::CenterTop:
      case AreaAlign::CenterMiddle:
      case AreaAlign::CenterBottom: return HorizontalAlign::Center;
      case AreaAlign::RightTop:
      case AreaAlign::RightMiddle:
      case AreaAlign::RightBottom: return HorizontalAlign::Right;
    }
    abort();
  }

  VerticalAlign toVerticalAlign(AreaAlign areaAlign)
  {
    switch (areaAlign)
    {
      case AreaAlign::LeftTop:
      case AreaAlign::CenterTop:
      case AreaAlign::RightTop: return VerticalAlign::Top;
      case AreaAlign::LeftMiddle:
      case AreaAlign::CenterMiddle:
      case AreaAlign::RightMiddle: return VerticalAlign::Center;
      case AreaAlign::LeftBottom:
      case AreaAlign:: CenterBottom:
      case AreaAlign::RightBottom: return VerticalAlign::Bottom;
    }
    abort();
  }

  AreaAlign combine(HorizontalAlign horizontalAlign, VerticalAlign verticalAlign)
  {
    switch (horizontalAlign)
    {
      case HorizontalAlign::Left:
        switch (verticalAlign)
        {
          case VerticalAlign::Top: return AreaAlign::LeftTop;
          case VerticalAlign::Center: return AreaAlign::LeftMiddle;
          case VerticalAlign::Bottom: return AreaAlign::LeftBottom;
        }
        break;
      case HorizontalAlign::Center:
        switch (verticalAlign)
        {
          case VerticalAlign::Top: return AreaAlign::CenterTop;
          case VerticalAlign::Center: return AreaAlign::CenterMiddle;
          case VerticalAlign::Bottom: return AreaAlign::CenterBottom;
        }
        break;
      case HorizontalAlign::Right:
        switch (verticalAlign)
        {
          case VerticalAlign::Top: return AreaAlign::RightTop;
          case VerticalAlign::Center: return AreaAlign::RightMiddle;
          case VerticalAlign::Bottom: return AreaAlign::RightBottom;
        }
        break;
    }
    abort();
  }

  HorizontalAlign parseHorizontalAlignFromString(std::string_view input)
  {
    if (input == "left")
      return HorizontalAlign::Left;
    if (input == "center")
      return HorizontalAlign::Center;
    if (input == "right")
      return HorizontalAlign::Right;
    throw std::runtime_error("\"" + std::string(input) + "\" isn't correct align value (put left, center or right)");
  }

  const char* horizontalAlignToString(HorizontalAlign value)
  {
    switch (value)
    {
      case HorizontalAlign::Left: return "left";
      case HorizontalAlign::Center: return "center";
      case HorizontalAlign::Right: return "right";
    }
    abort();
  }

  VerticalAlign parseVertialAlignFromString(std::string_view input)
  {
    if (input == "top")
      return VerticalAlign::Top;
    if (input == "center")
      return VerticalAlign::Center;
    if (input == "bottom")
      return VerticalAlign::Bottom;
    throw std::runtime_error("\"" + std::string(input) + "\" isn't correct vertical_align value (put top, center or bottom)");
  }

  const char* verticalAlignToString(VerticalAlign value)
  {
    switch (value)
    {
      case VerticalAlign::Top: return "top";
      case VerticalAlign::Center: return "center";
      case VerticalAlign::Bottom: return "bottom";
    }
    abort();
  }
}

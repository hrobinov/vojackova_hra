#include "Agui/AreaAlignmentEnum.hpp"
#include <stdexcept>

namespace agui
{
  // Refer to Gui/Style/AreaAlignments.cpp for lua docs

  AreaAlign parseAreaAlignmentFromString(std::string_view input)
  {
    if (input == "center")
      return AreaAlign::CenterTop;
    if (input == "left")
      return AreaAlign::LeftTop;
    if (input == "right")
      return AreaAlign::RightTop;

    if (input == "top-left")
      return AreaAlign::LeftTop;
    if (input == "middle-left")
      return AreaAlign::LeftMiddle;
    if (input == "bottom-left")
      return AreaAlign::LeftBottom;
    if (input == "top-center")
      return AreaAlign::CenterTop;
    if (input == "middle-center")
      return AreaAlign::CenterMiddle;
    if (input == "bottom-center")
      return AreaAlign:: CenterBottom;
    if (input == "top-right")
      return AreaAlign::RightTop;
    if (input == "middle-right")
      return AreaAlign::RightMiddle;
    if (input == "bottom-right")
      return AreaAlign::RightBottom;
    throw std::runtime_error("Unknown align type " + std::string(input));
  }

  const char* areaAlignemntEnumToString(AreaAlign value)
  {
    switch (value)
    {
      case AreaAlign::LeftTop: return "top-left";
      case AreaAlign::LeftMiddle: return "middle-left";
      case AreaAlign::LeftBottom: return "bottom-left";
      case AreaAlign::CenterTop: return "top-center";
      case AreaAlign::CenterMiddle: return "middle-center";
      case AreaAlign::CenterBottom: return "bottom-center";
      case AreaAlign::RightTop: return "top-right";
      case AreaAlign::RightMiddle: return "middle-right";
      case AreaAlign::RightBottom: return "bottom-right";
    }

    throw std::runtime_error("Unknown value.");
  }
}

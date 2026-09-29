#include "Agui/GuiDirection.hpp"
#include <stdexcept>

namespace agui
{
  std::string GuiDirection::str() const
  {
    switch (this->value)
    {
      case GuiDirection::Horizontal: return "horizontal";
      case GuiDirection::Vertical: return "vertical";
      default: throw std::runtime_error("Invalid internal gui direction value.");
    }
  }

  GuiDirection GuiDirection::parse(const std::string& value)
  {
    if (value == "horizontal")
      return GuiDirection::Horizontal;
    if (value == "vertical")
      return GuiDirection::Vertical;
    throw std::runtime_error("unknown flow GuiDirection value: " + value);
  }
}

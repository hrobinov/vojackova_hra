#include "Agui/ScrollPolicy.hpp"
#include <stdexcept>

namespace agui
{
  ScrollPolicy ScrollPolicy::parse(std::string_view input)
  {
    if (input == "never")
      return ScrollPolicy::Never;
    if (input == "dont-show-but-allow-scrolling")
      return ScrollPolicy::DontShowButAllowScrolling;
    if (input == "always")
      return ScrollPolicy::Always;
    if (input == "auto")
      return ScrollPolicy::Auto;
    if (input == "auto-and-reserve-space")
      return ScrollPolicy::AutoAndReserveSpace;
    throw std::runtime_error("\"" + std::string(input) + "\" is invalid scroll policy value");
  }

  const char* ScrollPolicy::str() const
  {
    switch (this->value)
    {
      case ScrollPolicy::Never: return "never";
      case ScrollPolicy::DontShowButAllowScrolling: return "dont-show-but-allow-scrolling";
      case ScrollPolicy::Always: return "always";
      case ScrollPolicy::Auto: return "auto";
      case ScrollPolicy::AutoAndReserveSpace: return "auto-and-reserve-space";
    }
    throw std::runtime_error("Invalid scroll policy value");
  }
}

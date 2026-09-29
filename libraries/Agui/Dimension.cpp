#include "Agui/Dimension.hpp"
#include <Agui/StringUtil.hpp>

namespace agui
{
  void Dimension::set(int width, int height)
  {
    this->width = width;
    this->height = height;
  }

  void Dimension::set(float width, float height)
  {
    this->width = (int)width;
    this->height = (int)height;
  }

  std::string Dimension::str() const
  {
    return ssprintf("{%i, %i}", this->width, this->height);
  }
}

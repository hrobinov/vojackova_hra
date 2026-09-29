#include "Agui/Point.hpp"
#include <Agui/Rectangle.hpp>
#include <Agui/StringUtil.hpp>
#include <Agui/Math.hpp>

namespace agui
{
  Point::Point(int x, int y)
   : x(x)
   , y(y)
  {}

  Point::Point(const Point& a, const Point& b, int x, int y)
    : x(a.x + b.x + x)
    , y(a.y + b.y + y)
  {}

  std::string Point::str() const
  {
    return ssprintf("{%i, %i}", this->x, this->y);
  }

  double Point::distance(const Point& other) const
  {
    const double deltaX = this->x - other.x;
    const double deltaY = this->y - other.y;
    return Math::sqrt(deltaX * deltaX + deltaY * deltaY);
  }

  void Point::clampTo(Rectangle rectangle)
  {
    this->x = Math::clamp(this->x, rectangle.getLeft(), rectangle.getRight());
    this->y = Math::clamp(this->y, rectangle.getTop(), rectangle.getBottom());
  }
}

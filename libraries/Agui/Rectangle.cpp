#include <Agui/Rectangle.hpp>
#include <Agui/Math.hpp>
#include <Agui/StringUtil.hpp>
#include <algorithm>

namespace agui
{
  Rectangle::Rectangle(int x, int y, int width, int height)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
  {}

  Rectangle::Rectangle(Point location, Dimension size)
    : x(location.x)
    , y(location.y)
    , width(size.width)
    , height(size.height)
  {}

  Rectangle::Rectangle(Point leftTop, Point rightBottom)
    : x(leftTop.x)
    , y(leftTop.y)
    , width(rightBottom.x - leftTop.x)
    , height(rightBottom.y - leftTop.y)
  {}

  int Rectangle::getTop() const
  {
    return this->y;
  }

  int Rectangle::getLeft() const
  {
    return this->x;
  }

  int Rectangle::getBottom() const
  {
    return this->y + this->height;
  }

  int Rectangle::getRight() const
  {
    return this->x + this->width;
  }

  int Rectangle::getCenterX() const
  {
    return this->x + this->width / 2;
  }

  int Rectangle::getCenterY() const
  {
    return this->y + this->height / 2;
  }

  void Rectangle::clear()
  {
    *this = Rectangle();
  }

  bool Rectangle::empty() const
  {
    return this->width == 0 && this->height == 0;
  }

  bool Rectangle::collide(Rectangle other) const
  {
    if (this->x > other.getRight())
      return false;
    if (this->y > other.getBottom())
      return false;
    if (this->getRight() < other.x)
      return false;
    if (this->getBottom() < other.y)
      return false;

    return true;
  }

  bool Rectangle::collide(Point point) const
  {
    if (this->x > point.x)
      return false;
    if (this->y > point.y)
      return false;
    if (this->getRight() < point.x)
      return false;
    if (this->getBottom() < point.y)
      return false;

    return true;
  }

  bool Rectangle::contains(Rectangle other) const
  {
    return other.x >= this->x &&
           other.y >= this->y &&
           other.getRight() <= this->getRight() &&
           other.getBottom() <= this->getBottom();
  }

  Point Rectangle::getLeftTop() const
  {
    return Point(getLeft(), getTop());
  }

  Point Rectangle::getRightBottom() const
  {
    return Point(getRight(), getBottom());
  }

  Point Rectangle::getCenter() const
  {
    return Point(getCenterX(), getCenterY());
  }

  Rectangle Rectangle::operator+(Point other) const
  {
    Rectangle result = *this;
    result.x += other.x;
    result.y += other.y;
    return result;
  }

  Rectangle Rectangle::operator-(Point other) const
  {
    Rectangle result = *this;
    result.x -= other.x;
    result.y -= other.y;
    return result;
  }

  std::string Rectangle::str() const
  {
    return ssprintf("{{%d, %d}, {%d, %d}}", this->x, this->y, this->width, this->height);
  }

  bool Rectangle::pointInside(const Point& p) const
  {
    if (p.x < this->x)
      return false;
    if (p.y < this->y)
      return false;
    if (p.x >= this->x + this->width)
      return false;
    if (p.y >= this->y + this->height)
      return false;
    return true;
  }

  void Rectangle::operator|=(const Rectangle& other)
  {
    int left = Math::min(this->getLeft(), other.getLeft());
    int right = Math::max(this->getRight(), other.getRight());
    int top = Math::min(this->getTop(), other.getTop());
    int bottom = Math::max(this->getBottom(), other.getBottom());
    *this = Rectangle(left, top, right - left, bottom - top);
  }

  Rectangle Rectangle::operator&(const Rectangle& other) const
  {
    int left = Math::max(this->getLeft(), other.getLeft());
    int right = Math::min(this->getRight(), other.getRight());
    int top = Math::max(this->getTop(), other.getTop());
    int bottom = Math::min(this->getBottom(), other.getBottom());

    if (left >= right || top >= bottom)
      return Rectangle();
    else
      return Rectangle(left, top, right - left, bottom - top);
  }

  void Rectangle::operator&=(const Rectangle& other)
  {
    *this = (*this & other);
  }

  int Rectangle::getWidth() const
  {
    return this->width;
  }

  int Rectangle::getHeight() const
  {
    return this->height;
  }

  Point Rectangle::getTopRight() const
  {
    return Point(this->getRight(), this->getTop());
  }

  Point Rectangle::getBottomLeft() const
  {
    return Point(this->getLeft(), this->getBottom());
  }

  Dimension Rectangle::getSize() const
  {
    return Dimension(this->width, this->height);
  }
}

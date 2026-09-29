#pragma once
#include <Agui/Dimension.hpp>
#include <string>
#include <limits>

namespace agui
{
  class Rectangle;

  /** Used for any integer Point / coordinate. */
  class Point
  {
  public:
    Point() = default;
    Point(int x, int y);
    Point(const Point& a, const Point& b, int x, int y);
    std::string str() const; // @return "{X,Y}"
    bool operator==(const Point& point) const { return this->x == point.x && this->y == point.y; }
    bool operator!=(const Point& point) const { return this->x != point.x || this->y != point.y; }
    void operator+=(const Point& point) { this->x += point.x; this->y += point.y; }
    Point operator+(const Dimension& size) const { return Point(this->x + size.width, this->y + size.height); }
    Point operator+(const Point& point) const { return Point(this->x + point.x, this->y + point.y); }
    Point operator-(const Point& point) const { return Point(this->x - point.x, this->y - point.y); }
    Point operator/(double divider) const { return Point(int(this->x / divider), int(this->y / divider)); }
    bool empty() const { return this->x == std::numeric_limits<int32_t>::max(); }
    double distance(const Point& other) const;
    void clampTo(Rectangle rectangle);

    static Point emptyPoint()
    {
      return Point(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
    }

    int x = 0;
    int y = 0;
  };
}

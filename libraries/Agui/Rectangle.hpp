#pragma once
#include "Agui/Point.hpp"
#include "Agui/Dimension.hpp"
namespace agui
{
  /** Integer Rectangle. Also used for clipping rectangles.*/
  class Rectangle
  {
  public:
    Rectangle() = default;
    Rectangle(int x, int y, int width, int height);
    Rectangle(Point leftTop, Point rightBottom);
    Rectangle(Point location, Dimension size);
    Rectangle operator&(const Rectangle& other) const;
    void operator&=(const Rectangle& other);
    void operator|=(const Rectangle& other);
    void operator+=(const Point& point) { this->x += point.x; this->y += point.y; }
    bool operator==(const Rectangle& other) const { return this->x == other.x && this->y == other.y && this->width == other.width && this->height == other.height; }
    std::string str() const;

    bool pointInside(const Point& p) const;
    int getWidth() const;
    int getHeight() const;
    int getTop() const;
    int getLeft() const;
    int getBottom() const;
    int getRight() const;
    int getCenterX() const;  /// rounded down (for odd width)
    int getCenterY() const;  /// rounded down (for odd height)
    void clear();
    bool empty() const;
    bool collide(Rectangle other) const;
    bool contains(Rectangle other) const;
    bool collide(Point point) const;

    Dimension getSize() const;
    Point getLeftTop() const;
    Point getTopRight() const;
    Point getBottomLeft() const;
    Point getRightBottom() const;
    Point getCenter() const;  /// rounded down (for odd width or height)

    Rectangle operator+(Point other) const;
    Rectangle operator-(Point other) const;

    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
  };
}

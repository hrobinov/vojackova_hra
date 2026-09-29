#pragma once
#include <string>
namespace agui
{
  /** Used for the size / dimensions of widgets and other objects. */
  class Dimension
  {
  public:
    Dimension(int width, int height) : width(width), height(height) {}
    Dimension() = default;
    void set(int width, int height);
    void set(float width, float height); /**< Floats will be interpreted as ints. */
    std::string str() const; /**< @return "{WIDTH,HEIGHT}". */
    bool operator==(const Dimension& other) const = default;
    bool operator!=(const Dimension& other) const = default;

    int width = 0;
    int height = 0;
  };
}

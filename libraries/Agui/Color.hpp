#pragma once
#include <string>

namespace agui
{
  /** Uses floating point precision. */
  class Color
  {
    void verifyColorBounds(); // Ensures that colors are in the correct range.
  public:
    static bool isAlphaPremultiplied(); // If the RGB components of the color should be multiplied by the A component.
    static void setPremultiplyAlpha(bool premultiply); // Sets if the RGB components of the color should be multiplied by the A component.
    Color(float r, float g, float b, float a); // Construct a color using, Red, Green, Blue, Alpha values ranging from 0.0 to 1.0
    Color(float r, float g, float b); // Construct a color using, Red, Green, Blue values ranging from 0.0 to 1.0.
    Color() = default;
    float getR() const;
    float getG() const;
    float getB() const;
    float getA() const;
    void setR(float r) { this->r = r; this->verifyColorBounds(); }
    void setG(float g) { this->g = g; this->verifyColorBounds(); }
    void setB(float b) { this->b = b; this->verifyColorBounds(); }
    void setA(float a) { this->a = a; this->verifyColorBounds(); }
    std::string str() const;
    bool operator==(const Color&) const = default;
    bool operator!=(const Color&) const = default;
    Color operator*(const Color& other) const;
  private:
    static bool premultiplyAlpha;
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;
  };
}

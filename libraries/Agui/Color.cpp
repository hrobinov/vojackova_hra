#include "Agui/Color.hpp"
#include <Agui/StringUtil.hpp>

namespace agui
{
  bool Color::premultiplyAlpha = false;

  Color::Color(float r, float g, float b, float a)
  {
    if (Color::premultiplyAlpha)
    {
      this->r = r * a;
      this->g = g * a;
      this->b = b * a;
      this->a = a;
    }
    else
    {
      this->r = r;
      this->g = g;
      this->b = b;
      this->a = a;
    }
    verifyColorBounds();
  }

  Color::Color(float r, float g, float b)
  {
    *this = Color(r, g, b, 1.0f);
  }

  void Color::verifyColorBounds()
  {
    if (this->r > 1.0f)
      this->r = 1.0f;
    if (this->r < 0.0f)
      this->r = 0.0f;

    if (this->g > 1.0f)
      this->g = 1.0f;
    if (this->g < 0.0f)
      this->g = 0.0f;

    if (this->b > 1.0f)
      this->b = 1.0f;
    if (this->b < 0.0f)
      this->b = 0.0f;

    if (this->a > 1.0f)
      this->a = 1.0f;
    if (this->a < 0.0f)
      this->a = 0.0f;
  }

  float Color::getR() const
  {
    return this->r;
  }

  float Color::getG() const
  {
    return this->g;
  }

  float Color::getB() const
  {
    return this->b;
  }

  float Color::getA() const
  {
    return this->a;
  }

  bool Color::isAlphaPremultiplied()
  {
    return Color::premultiplyAlpha;
  }

  void Color::setPremultiplyAlpha(bool premultiply)
  {
    Color::premultiplyAlpha = premultiply;
  }

  std::string Color::str() const
  {
    if (this->a != 1)
      return ssprintf("{%g, %g, %g, %g}", this->r, this->g, this->b, this->a);
    return ssprintf("{%g, %g, %g}", this->r, this->g, this->b);
  }

  Color Color::operator*(const Color& other) const
  {
    return Color(this->r * other.r,
                 this->g * other.g,
                 this->b * other.b,
                 this->a * other.a);
  }
}

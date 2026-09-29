#pragma once
#include <Agui/FlyingText.hpp>
#include <Agui/Point.hpp>

namespace agui
{
  class Graphics;
  class Font;

  class GuiFlyingText : public FlyingText
  {
  public:
    GuiFlyingText(Point position, std::string text, Color color, const Font* font, uint32_t timeToLive, int screenWidth);
    void paint(Graphics* graphicsContext) const;
    Point getCurrentPosition() const;
  private:
    void checkThatItIsOnScreen(Point& position) const;

  public:
    Point startingPosition;
  };
}

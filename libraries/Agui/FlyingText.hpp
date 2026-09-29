#pragma once
#include <Agui/Color.hpp>
#include <cstdint>

namespace agui
{
  class Font;

  class FlyingText
  {
  protected:
    FlyingText(std::string&& text, Color color, const Font* font, uint32_t timeToLive, float customSpeed = -1);
    float getShiftY(float scale) const;
  public:
    bool update(); // Returns true if it should be removed.
    static float calculateSpeed(std::string_view text, const Font* font);

    std::string text;
    Color color;
    const Font* font = nullptr;
    float speed;
    uint32_t timeToLive;
    uint32_t age = 0;
  };
}

#include <Agui/FlyingText.hpp>
#include <Agui/Font.hpp>
#include <Agui/Math.hpp>
#include <Agui/StringUtil.hpp>

namespace agui
{
  FlyingText::FlyingText(std::string&& text, Color color, const Font* font, uint32_t timeToLive, float customSpeed)
    : text(std::move(text))
    , color(color)
    , font(font)
    , timeToLive(timeToLive)
  {
    if (customSpeed == -1)
    {
      this->speed = FlyingText::calculateSpeed(this->text, this->font);
      this->timeToLive /= this->speed;
    }
    else
      this->speed = customSpeed / 60;
  }

  float FlyingText::getShiftY(float scale) const
  {
    return -this->speed / scale * this->age;
  }

  float FlyingText::calculateSpeed(std::string_view text, const Font* font)
  {
    static constexpr int BASE_WIDTH = 155;
    const int textWidth = font->getTextWidth(text, RichTextSetting::Enabled);
    float speed = textWidth > BASE_WIDTH ? float(BASE_WIDTH) / textWidth : 1;
    return Math::max(speed, 0.5f);
  }

  bool FlyingText::update()
  {
    return ++this->age >= this->timeToLive;
  }
}

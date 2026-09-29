#include <Agui/Font.hpp>
#include <Agui/Graphics.hpp>
#include <Agui/Gui.hpp>
#include <Agui/GuiFlyingText.hpp>

namespace agui
{
  GuiFlyingText::GuiFlyingText(Point position, std::string text, Color color, const Font* font, uint32_t timeToLive, int screenWidth)
    : FlyingText(std::move(text), color, font, timeToLive)
    , startingPosition(position)
  {
    const int32_t width = this->font->getTextWidth(this->text, RichTextSetting::Enabled);
    this->startingPosition.x -= width / 2;
    if (this->startingPosition.x + width > screenWidth)
      this->startingPosition.x = std::max(screenWidth - width, 0);

    this->startingPosition.y -= this->font->getLineHeight();
    this->checkThatItIsOnScreen(this->startingPosition);
  }

  void GuiFlyingText::paint(Graphics* graphicsContext) const
  {
    graphicsContext->drawTextLines(this->getCurrentPosition(), this->text, this->color, this->font, agui::RichTextSetting::Enabled);
  }

  Point GuiFlyingText::getCurrentPosition() const
  {
    Point position(this->startingPosition.x, this->startingPosition.y + this->getShiftY(1 / Gui::scale));
    this->checkThatItIsOnScreen(position);
    return position;
  }

  void GuiFlyingText::checkThatItIsOnScreen(Point& position) const
  {
    int lineHeight = this->font->getLineHeight();
    if (position.y < lineHeight)
      position.y = lineHeight;
    if (position.x < 0)
      position.x = 0;
  }
}

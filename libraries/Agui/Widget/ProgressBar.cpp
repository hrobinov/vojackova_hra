#include "Agui/Font.hpp"
#include "Agui/Widget/ProgressBar.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include <Agui/Math.hpp>
#include <algorithm>
#include <cassert>

namespace agui
{
  ProgressBarStyle ProgressBar::defaultStyle;

  ProgressBar::ProgressBar(const ProgressBarStyle* parentStyle, GuiDirection direction, HasText hasText)
    : style(this, parentStyle)
    , direction(direction)
    , hasText(hasText == HasText::Yes)
  {}

  void ProgressBar::paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    Rectangle fullRectangle = this->getBarRectangle();

    Color color = this->style.getColor();
    const ElementImageSet* bar = this->style.getBar();
    for (const ProgressBarStyle::OtherColor& differentColor : this->style.getOtherColors())
      if (this->value < differentColor.valueLessThan)
      {
        if (differentColor.color)
          color = *differentColor.color;
        if (differentColor.bar)
          bar = &*differentColor.bar;
        break;
      }

    // The progress bar - only the complete fraction of length
    Rectangle clippedRectangle;
    if (this->direction == GuiDirection::Horizontal)
      clippedRectangle = Rectangle(0,
                                   this->reserveSpaceForText() ? this->style.getFont()->getLineHeight() : 0,
                                   int(this->getContentWidth() * this->value),
                                   this->style.getBarWidth());
    else
      clippedRectangle = Rectangle(0,
                                   int(this->getContentHeight() * (1.0 - this->value)),
                                   this->style.getBarWidth(),
                                   this->getContentHeight());

    paintEvent.graphics()->pushClippingRect(this, clippedRectangle, true);
    bar->base.draw(paintEvent, fullRectangle, absolutePosition, 1, color);
    paintEvent.graphics()->popClippingRect();

    if (!this->getText().empty())
    {
      Point topLeft;

      topLeft.y = this->style.getEmbedTextInBar()
                  ? fullRectangle.getHeight() / 2 - this->style.getFont()->getLineHeight() / 2
                  : this->style.getTopPadding();

      const int textWidth = this->style.getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled);

      if (this->style.getHorizontalAlign() == HorizontalAlign::Left)
        topLeft.x = this->style.getSideTextPadding();
      else if (this->style.getHorizontalAlign() == HorizontalAlign::Center)
        topLeft.x = fullRectangle.getWidth() / 2 - textWidth / 2;
      else if (this->style.getHorizontalAlign() == HorizontalAlign::Right)
        topLeft.x = fullRectangle.getWidth() - textWidth - this->style.getSideTextPadding();

      paintEvent.graphics()->drawText(topLeft,
                                      this->getText(),
                                      this->style.getFontColor(), this->style.getFont(),
                                      RichTextSetting::Enabled);

      if (const Color* filledColor = this->style.getFilledFontColor())
      {
        paintEvent.graphics()->pushClippingRect(this, clippedRectangle, true);
        paintEvent.graphics()->drawText(topLeft,
                                        this->getText(),
                                        *filledColor, this->style.getFont(),
                                        RichTextSetting::Enabled);
        paintEvent.graphics()->popClippingRect();
      }
    }
  }

  void ProgressBar::paintBackground(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getBackground()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void ProgressBar::paintBackgroundShadow(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    // note - while painCompoment is called with offset already including paddings, this is not.
    Rectangle barRectangle = this->getBarRectangle();
    barRectangle.x += this->getLeftPadding();
    barRectangle.y += this->getTopPadding();
    this->style.getBackground()->shadow.draw(paintEvent, barRectangle, absolutePosition);
  }

  void ProgressBar::resizeToContents()
  {
    if (this->direction == agui::GuiDirection::Horizontal)
    {
      int width;
      if (this->isHorizontallyStretchable())
        width = this->style.getMinimalWidth();
      else
        width = this->hasText ?
                  std::max(this->style.getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled), this->style.getNaturalWidth()) :
                  this->style.getNaturalWidth();
      this->setContentSize(width, (this->reserveSpaceForText() ? this->style.getFont()->getLineHeight() : 0) + this->style.getBarWidth());
    }
    else
      this->setContentSize(this->style.getBarWidth(), this->style.getMinimalHeight());
  }

  void ProgressBar::setValue(double value)
  {
    this->value = Math::clamp(value, double(0), double(1));
  }

  Rectangle ProgressBar::getBarRectangle() const
  {
    if (this->direction == GuiDirection::Horizontal)
      return Rectangle(0,
                       this->reserveSpaceForText() ? this->style.getFont()->getLineHeight() : 0,
                       this->getContentWidth(),
                       this->style.getBarWidth());
    else
      return Rectangle(0, 0, this->style.getBarWidth(), this->getContentHeight());
  }

  bool ProgressBar::reserveSpaceForText() const
  {
    return this->hasText && !this->style.getEmbedTextInBar();
  }
}

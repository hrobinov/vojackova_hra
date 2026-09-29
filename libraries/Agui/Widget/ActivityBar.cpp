#include "Agui/Widget/ActivityBar.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"

namespace agui
{
  ActivityBarStyle ActivityBar::defaultStyle;

  ActivityBar::ActivityBar(const ActivityBarStyle* parentStyle)
    : style(this, parentStyle)
  {}

  void ActivityBar::paintComponent(const agui::PaintEvent &paintEvent, const agui::Point &absolutePosition)
  {
    Rectangle clippedRectangle = Rectangle(this->getContentWidth() * this->leftPositionRatio,
                                           0,
                                           this->getContentWidth() * (this->rightPositionRatio - this->leftPositionRatio),
                                           this->style.getBarWidth());

    paintEvent.graphics()->pushClippingRect(this, clippedRectangle, true);
    this->style.getBar()->base.draw(paintEvent, this->getBarRectangle(), absolutePosition, 1, this->style.getColor());
    paintEvent.graphics()->popClippingRect();
  }

  void ActivityBar::paintBackground(const agui::PaintEvent &paintEvent, const agui::Point &absolutePosition)
  {
    this->style.getBackground()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void ActivityBar::paintBackgroundShadow(const agui::PaintEvent &paintEvent, const agui::Point &absolutePosition)
  {
    // note - while painCompoment is called with offset already including paddings, this is not.
    Rectangle barRectangle = this->getBarRectangle();
    barRectangle.x += this->getLeftPadding();
    barRectangle.y += this->getTopPadding();
    this->style.getBackground()->shadow.draw(paintEvent, barRectangle, absolutePosition);
  }

  void ActivityBar::logic(double timeElapsed)
  {
    if (timeElapsed - this->lastUpdate > 1.0 / 60.0)
    {
      this->lastUpdate = timeElapsed;
      this->rightPositionRatio += this->style.getSpeed();
      if (this->rightPositionRatio > 1.0)
        this->rightPositionRatio = 1.0;
      if (this->rightPositionRatio > this->style.getBarSizeRatio())
        this->leftPositionRatio += this->style.getSpeed();
      if (this->leftPositionRatio >= 1.0)
      {
        this->leftPositionRatio = 0;
        this->rightPositionRatio = 0;
      }
    }
  }

  Rectangle ActivityBar::getBarRectangle() const
  {
    return Rectangle(0, 0, this->getContentWidth(), this->style.getBarWidth());
  }

  void ActivityBar::resizeToContents()
  {
    this->setContentSize(this->isHorizontallyStretchable() ? this->style.getMinimalWidth() : this->style.getNaturalWidth(),
                         this->style.getBarWidth());
  }
}

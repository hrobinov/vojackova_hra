#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Line.hpp"

namespace agui
{
  LineStyle Line::defaultStyle;

  Line::Line(GuiDirection direction, const LineStyle* parentStyle)
    : style(this, parentStyle)
    , direction(direction)
  {
    if (direction == GuiDirection::Horizontal)
      this->style.setHorizontallyStretchable();
    else
      this->style.setVerticallyStretchable();
  }

  void Line::resizeToContents()
  {
    if (direction == GuiDirection::Horizontal)
      this->setSize(this->isHorizontallyStretchable() ? 0 : this->style.getNaturalWidth(),
                    this->style.getBorder()->lineWidth + this->getVerticalPaddings());
    else
      this->setSize(this->style.getBorder()->lineWidth + this->getHorizontalPaddings(),
                    this->isVerticallyStretchable() ? 0 : this->style.getNaturalHeight());
  }

  void Line::paintComponent(const agui::PaintEvent& paintEvent, const agui::Point&)
  {
    if (!this->style.getBorder()->isSet)
      return;
    if (direction == GuiDirection::Horizontal)
      this->style.getBorder()->drawHorizontalLine(paintEvent, Point(0, (this->getHeight() - this->style.getBorder()->lineWidth) / 2), this->getContentWidth());
    else
      this->style.getBorder()->drawVerticalLine(paintEvent, Point((this->getWidth() - this->style.getBorder()->lineWidth) / 2 , 0), this->getContentHeight());
  }
}

#include <Agui/BorderImageSet.hpp>
#include <Agui/Graphics.hpp>
#include <Agui/Gui.hpp>
#include <Agui/Image.hpp>
#include <Agui/PaintEvent.hpp>
#include <cassert>
#include <stdexcept>

namespace agui
{
  const Image* BorderImageSet::getSprite(const Terminal type) const
  {
    if (!this->isSet)
      throw std::runtime_error("Attempting to request border sprite for object that does not use them");

    switch(type)
    {
      case Terminal::TopRightCorner: return this->topRightCorner; break;
      case Terminal::BottomRightCorner: return this->bottomRightCorner; break;
      case Terminal::BottomLeftCorner: return this->bottomLeftCorner; break;
      case Terminal::TopLeftCorner: return this->topLeftCorner; break;
      case Terminal::TopT: return this->topT; break;
      case Terminal::RightT: return this->rightT; break;
      case Terminal::BottomT: return this->bottomT; break;
      case Terminal::LeftT: return this->leftT; break;
      case Terminal::Cross: return this->cross; break;
      case Terminal::TopEnd: return this->topEnd; break;
      case Terminal::RightEnd: return this->rightEnd; break;
      case Terminal::BottomEnd: return this->bottomEnd; break;
      case Terminal::LeftEnd: return this->leftEnd; break;
      default:
        throw std::runtime_error("Attempting to request non existant sprite");
    }
  }

  void BorderImageSet::drawTerminal(const PaintEvent& paintEvent, const Point position, const Terminal terminal) const
  {
    if (!this->isSet)
      return;

    if (terminal == Terminal::None || terminal == Terminal::Empty)
      return;

    if (const agui::Image* terminalSprite = this->getSprite(terminal))
      paintEvent.graphics()->drawScaledImage(terminalSprite, position, Dimension(this->lineWidth, this->lineWidth));
  }

  void BorderImageSet::drawHorizontalLine(const PaintEvent & paintEvent, const Point start, const int length, const Terminal left, const Terminal right) const
  {
    if (!this->isSet)
      return;

    if (length <= 0)
      return;

    // check that the ends match and that the line is long enough for what is requested
    assert(left == Terminal::LeftEnd ||
           left == Terminal::LeftT ||
           left == Terminal::TopLeftCorner ||
           left == Terminal::BottomLeftCorner ||
           left == Terminal::None ||
           left == Terminal::Empty);
    assert(right == Terminal::RightEnd ||
           right == Terminal::RightT ||
           right == Terminal::TopRightCorner ||
           right == Terminal::BottomRightCorner ||
           right == Terminal::None ||
           right == Terminal::Empty);

    int middleLength = length - (left == Terminal::None ? 0 : this->lineWidth) - (right == Terminal::None ? 0 : this->lineWidth);

    int position = start.x;
    // draw top end
    if (left != Terminal::None)
    {
      if (left != Terminal::Empty)
        if (const agui::Image* leftSprite = this->getSprite(left))
          paintEvent.graphics()->drawScaledImage(leftSprite, start, Dimension(this->lineWidth, this->lineWidth));
      position += this->lineWidth;
    }
    // draw middle part
    if (middleLength > 0 && this->horizontalLine)
      paintEvent.graphics()->drawScaledImage(this->horizontalLine, Point(position, start.y), Dimension(middleLength, this->lineWidth));
    // draw bottom end
    if (right != Terminal::None && right != Terminal::Empty)
      if (const agui::Image* rightSprite = this->getSprite(right))
        paintEvent.graphics()->drawScaledImage(rightSprite, Point(position + middleLength, start.y), Dimension(this->lineWidth, this->lineWidth));
  }

  void BorderImageSet::drawVerticalLine(const PaintEvent& paintEvent, const Point start, const int length, const Terminal top, const Terminal bottom) const
  {
    if (!this->isSet)
      return;

    if (length <= 0)
      return;

    // check that the ends match and that the line is long enough for what is requested
    assert(top == Terminal::TopEnd ||
           top == Terminal::TopT ||
           top == Terminal::TopRightCorner ||
           top == Terminal::TopLeftCorner ||
           top == Terminal::None ||
           top == Terminal::Empty);
    assert(bottom == Terminal::BottomEnd ||
           bottom == Terminal::BottomT ||
           bottom == Terminal::BottomRightCorner ||
           bottom == Terminal::BottomLeftCorner ||
           bottom == Terminal::None ||
           bottom == Terminal::Empty);

    int middleLength = length - (top == Terminal::None ? 0 : this->lineWidth) - (bottom == Terminal::None ? 0 : this->lineWidth);
    assert(middleLength >= 0);

    int position = start.y;
    // draw top end
    if (top != Terminal::None)
    {
      if (top != Terminal::Empty)
        if (const agui::Image* topSprite = this->getSprite(top))
          paintEvent.graphics()->drawScaledImage(topSprite, start, Dimension(this->lineWidth, this->lineWidth));
      position += this->lineWidth;
    }
    // draw middle part
    if (middleLength > 0 && this->verticalLine)
      paintEvent.graphics()->drawScaledImage(this->verticalLine, Point(start.x, position), Dimension(this->lineWidth, middleLength));
    // draw bottom end
    if (bottom != Terminal::None && bottom != Terminal::Empty)
      if (const agui::Image* bottomSprite = this->getSprite(bottom))
        paintEvent.graphics()->drawScaledImage(bottomSprite, Point(start.x, position + middleLength), Dimension(this->lineWidth, this->lineWidth));
  }

  void BorderImageSet::drawBorder(const PaintEvent& paintEvent, const Point start, const Dimension size) const
  {
    if (!this->isSet)
      return;
    if (size.width < this->lineWidth || size.height < lineWidth)
      return;

    // left side
    this->drawVerticalLine(paintEvent, start, size.height, Terminal::TopLeftCorner, Terminal::BottomLeftCorner);
    // right side
    this->drawVerticalLine(paintEvent, Point(start.x + size.width - this->lineWidth, start.y), size.height, Terminal::TopRightCorner, Terminal::BottomRightCorner);
    // top
    this->drawHorizontalLine(paintEvent, start, size.width, Terminal::Empty, Terminal::Empty);
    // bottom
    this->drawHorizontalLine(paintEvent, Point(start.x, start.y + size.height - this->lineWidth), size.width, Terminal::Empty, Terminal::Empty);
  }
}

#include "Agui/Graphics.hpp"
#include "Agui/ResizableText.hpp"
#include <Agui/Widget.hpp>
#include <cassert>
#include <stdint.h>

namespace agui
{
  const Rectangle Graphics::FULL_SCREEN_RECTANGLE = Rectangle(-1, 0, 0, 0);

  bool Graphics::isClippingRectEmpty()
  {
    return this->clipRectangle.getLeft() >= this->clipRectangle.getRight() || this->clipRectangle.getTop() >= this->clipRectangle.getBottom();
  }

  bool Graphics::pushClippingRect(const agui::Widget* widget, const Rectangle rect, bool force, const agui::Point* customOffset)
  {
    Rectangle relativeRectangle = Rectangle(rect.x + (customOffset ? customOffset->x : this->getOffset().x),
                                            rect.y + (customOffset ? customOffset->y : this->getOffset().y), rect.width, rect.height);

    if (this->clipStack.empty() || this->clipRectangle == FULL_SCREEN_RECTANGLE)
    {
      this->clipRectangle = relativeRectangle;
      this->clipStack.push_back({relativeRectangle, false, force});
      if (force || widget->shouldClipRendering())
      {
        this->clipStack.back().applied = true;
        this->setClippingRectangle(this->clipRectangle, force);
      }
      return !this->isClippingRectEmpty();
    }

    this->clipRectangle &= relativeRectangle;

    this->clipStack.push_back({this->clipRectangle, false, force});
    if (force || widget->shouldClipRendering())
    {
      this->clipStack.back().applied = true;
      this->setClippingRectangle(this->clipRectangle, force);
    }

    return !this->isClippingRectEmpty();
  }

  void Graphics::setForcedClippedRectangleToLastForced()
  {
    this->setClippingRectangle(this->getForcedRectangle(), true);
  }

  Rectangle Graphics::getForcedRectangle() const
  {
    for (int32_t i = int32_t(this->clipStack.size()) - 1; i >= 0; --i)
      if (this->clipStack[i].force)
        return this->clipStack[i].rectangle;
    return Graphics::FULL_SCREEN_RECTANGLE;
  }

  void Graphics::setClippingRectangleToTheLastForced()
  {
    this->clipRectangle = this->getForcedRectangle();
  }

  void Graphics::setClippedRectangleToLastApplied()
  {
    for (int32_t i = int32_t(this->clipStack.size()) - 1; i >= 0; --i)
      if (this->clipStack[i].applied)
      {
        this->setClippingRectangle(this->clipStack[i].rectangle, false);
        return;
      }
    this->setClippingRectangle(Graphics::FULL_SCREEN_RECTANGLE, false);
  }

  void Graphics::popClippingRect()
  {
    assert(!this->clipStack.empty());
    bool lastWasForced = this->clipStack.back().force;
    bool lastApplied = this->clipStack.back().applied;
    this->clipStack.pop_back();

    // When the last order was forced, we need to set the forced clipping state to the value
    // it was before this order was execute, and it was the value of the last forced command from the stack.
    if (lastWasForced)
      this->setForcedClippedRectangleToLastForced();
    if (lastApplied)
      this->setClippedRectangleToLastApplied();

    // The forced clipping rectangle is set correctly now, but the internal clipRect needs to be updated as well
    if (!this->clipStack.empty())
      this->clipRectangle = this->clipStack.back().rectangle;
  }

  void Graphics::clearClippingStack()
  {
    this->clipStack.clear();
    this->clipRectangle = Graphics::FULL_SCREEN_RECTANGLE;
    this->setClippingRectangle(clipRectangle, true);
  }

  const Point& Graphics::getOffset() const
  {
    return this->offset;
  }

  void Graphics::setOffset(const Point& offset)
  {
    this->offset = offset;
  }

  void Graphics::pushFullScreenClippingRect()
  {
    this->clipRectangle = Graphics::FULL_SCREEN_RECTANGLE;
    this->clipStack.push_back({ Graphics::FULL_SCREEN_RECTANGLE, true, true });
    this->setClippingRectangle(Graphics::FULL_SCREEN_RECTANGLE, true);
  }

  void Graphics::drawTextLines(const ResizableText& text,
                               std::vector<std::pair<size_t, agui::Point>>& linePositions,
                               const Font* font,
                               const Color& color,
                               const agui::TextHighlightErrorColors& highlightColors)
  {
    for (const auto& line : linePositions)
      this->drawText(line.second, std::string(text.lines()[line.first]), color, font, text.getRichTextSetting(), HorizontalAlign::Left, highlightColors);
  }
}

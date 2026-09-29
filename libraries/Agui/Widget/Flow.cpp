#include "Agui/Widget/ListBox.hpp"
#include "Agui/Widget/Flow.hpp"
#include <cassert>
#include <algorithm>

namespace agui
{
  FlowStyle Flow::defaultStyle;

  Flow::Flow(const FlowStyle* parentStyle)
    : style(this, parentStyle)
  {
    this->shrinkInReactionToSetSize();
  }

  Flow::~Flow() = default;

  void Flow::layoutChildren(SetSizeInfo setSizeInfo)
  {
    int x = 0, y = 0;
    std::vector<Widget*> currentRow;
    int highestWidget = 0;
    int maxX = 0;
    for (Widget* widget : VisibleChildren(this))
    {
      int right = x + widget->getWidth();
      if (!currentRow.empty() &&
          (this->style.getMaxOnRow() && currentRow.size() >= this->style.getMaxOnRow() ||
           this->style.getMaximalWidth() > 0 && right > this->style.getMaximalWidth() ||
           setSizeInfo.reactionToSetSize && right > this->getContentWidth()))
      {
        maxX = std::max(maxX, x - this->style.getHorizontalSpacing());
        this->finishRow(currentRow, highestWidget, x, y);
      }
      widget->setLocation(x, y);
      x += widget->getWidth() + this->style.getHorizontalSpacing();
      if (widget->getHeight() > highestWidget)
        highestWidget = widget->getHeight();
      currentRow.push_back(widget);
    }

    if (!currentRow.empty())
    {
      maxX = std::max(maxX, x - this->style.getHorizontalSpacing());
      this->finishRow(currentRow, highestWidget, x, y);
    }
    y -= this->style.getVerticalSpacing(); // last row vertical spacing correction

    if (this->canShrink(setSizeInfo.reactionToSetSize))
      this->setContentSizeInternal(maxX, y);
    else
      this->setContentSizeInternal(std::max(std::max(this->style.getNaturalWidth(), maxX), this->getContentWidth()),
                                   std::max(this->style.getNaturalHeight(), y));
    this->sizeBeforeStretching.height = this->getHeight();
  }

  int Flow::getContentsHeight() const
  {
    return contentsHeight;
  }

  int Flow::getContentsWidth() const
  {
    return this->getWidth() - this->getLeftPadding() - this->getRightPadding();
  }

  void Flow::resizeToContents()
  {
    if (this->isHorizontallyStretchable())
    {
      int highestWidget = 0;
      for (Widget* widget : VisibleChildren(this))
        highestWidget = std::max(highestWidget, widget->getHeight());
      Widget::setSize(0, highestWidget + this->getVerticalPaddings());
      return;
    }
    super::resizeToContents();
  }

  void Flow::finishRow(std::vector<Widget*>& currentRow, int& highestWidget, int& x, int& y)
  {
    assert(!currentRow.empty());

    if ((this->style.getVerticalAlign() == VerticalAlign::Center) && currentRow.size() > 1)
    {
      for (Widget* widget : currentRow)
      {
        int usableHeight = currentRow.size() > 1 ? highestWidget : this->getContentHeight();
        widget->setLocation(widget->getLocation().x,
                            widget->getLocation().y + (this->style.getVerticalAlign() == VerticalAlign::Center ? (std::max(usableHeight - widget->getHeight(), 0)) / 2 : 0));
      }
    }

    x = 0;
    y += highestWidget + this->style.getVerticalSpacing();
    highestWidget = 0;
    currentRow.clear();
  }

  int Flow::maximumHorizontalSquashSize() const
  {
    int result = 0;
    for (const Widget* child : ConstVisibleChildren(this))
      result += child->maximumHorizontalSquashSize();
    int lastWidgetRight = this->empty() ? 0 : this->getChildren().back()->getRightBorder();
    int contentWidth = this->getContentWidth();
    if (lastWidgetRight < contentWidth)
      result += contentWidth - lastWidgetRight;
    return result;
  }

  void Flow::setMinOnRow(uint32_t min)
  {
    this->minOnRow = min;
  }
}

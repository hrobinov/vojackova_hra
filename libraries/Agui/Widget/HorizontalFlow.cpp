#include "Agui/Widget/HorizontalFlow.hpp"
#include <Agui/SquashCalculator.hpp>
#include <Agui/Math.hpp>
#include <algorithm>

namespace agui
{
  HorizontalFlowStyle HorizontalFlow::defaultStyle;

  HorizontalFlow::HorizontalFlow(const HorizontalFlowStyle* parentStyle)
   : style(this, parentStyle ? parentStyle : &HorizontalFlow::defaultStyle)
  {
    this->shrinkInReactionToSetSize();
  }

  HorizontalFlow::HorizontalFlow(VerticalAlign align)
    : HorizontalFlow()
  {
    this->style.setVerticalAlign(align);
  }

  void HorizontalFlow::layoutChildren(SetSizeInfo setSizeInfo)
  {
    if (this->empty())
    {
      if (!setSizeInfo.reactionToSetSize)
        this->setContentSizeInternal(this->style.getNaturalWidth(), this->style.getNaturalHeight());
      return;
    }

    int x = 0;
    int targetContentHeight = 0;
    int horizontallyStretchableCount = 0;
    int horizontallySquashableCount = 0;
    bool childIncreasedWidthBySquashing = false;
    int extraHorizontalSpaceTakenByHorizontallyStretchable = 0;
    HorizontalAlign horizontalAlign = this->style.getHorizontalAlign();
    VerticalAlign verticalAlign = this->style.getVerticalAlign();

    if (!setSizeInfo.reactionToSetSize)
    {
      for (Widget* widget : VisibleChildren(this))
        targetContentHeight = std::max(targetContentHeight, widget->getHeight() + widget->getVerticalMargins());
      targetContentHeight = std::max(targetContentHeight, this->style.getMinimalHeight());
      if (this->style.getMaximalHeight() != 0)
        targetContentHeight = std::min(targetContentHeight, this->style.getMaximalHeight() - this->getVerticalPaddings());
    }
    else
      targetContentHeight = this->getContentHeight();

    // first layouting, widget are stretched to target height and positioned
    for (Widget* widget : VisibleChildren(this))
    {
      int widgetVerticalMargins = widget->getVerticalMargins();
      bool stretch = widget->getHeight() - widgetVerticalMargins < targetContentHeight && widget->isVerticallyStretchable();
      bool squash = false;
      int squashableSize = 0;
      if (widget->getHeight() - widgetVerticalMargins > targetContentHeight)
      {
        squashableSize = widget->maximumVerticalSquashSize();
        if (squashableSize > 0)
          squash = true;
      }

      int y = 0;
      if (stretch || squash)
      {
        int widthBefore = widget->getWidth();
        SetSizeInfo setSizeInfoParameter(Change::Nothing, stretch ? Change::Stretching : Change::Squashing);
        if (squash)
        {
          if (setSizeInfo.reactionToSetSize && setSizeInfo.horizontal == Change::Squashing)
            setSizeInfoParameter.maximalWidth = targetContentHeight;
          widget->setSizeForce(widget->getWidth(), std::max(targetContentHeight - widgetVerticalMargins, widget->getHeight() - squashableSize), setSizeInfoParameter);
        }
        else
          widget->setSizeForce(widget->getWidth(), targetContentHeight - widgetVerticalMargins, setSizeInfoParameter);
        if (widthBefore < widget->getWidth())
          childIncreasedWidthBySquashing = true;
        y = widget->getTopMargin();
      }
      else if (verticalAlign == VerticalAlign::Bottom)
        y = targetContentHeight - widget->getHeight() - widget->getBottomMargin();
      else if (verticalAlign == VerticalAlign::Center)
        y = (targetContentHeight - widget->getHeight()) / 2 + widget->getTopMargin();
      else
        y = widget->getTopMargin();

      x += widget->getLeftMargin();
      widget->setLocation(x, y);
      if (widget->isHorizontallyStretchable())
      {
        extraHorizontalSpaceTakenByHorizontallyStretchable += widget->getWidth() - widget->getSizeBeforeStretching().width;
        ++horizontallyStretchableCount;
      }
      horizontallySquashableCount += widget->isHorizontallySquashable();
      x += widget->getWidth() + this->style.getHorizontalSpacing();
      x += widget->getRightMargin();
    }

    x -= this->style.getHorizontalSpacing(); // compensate for the last item not having horizontal spacing on the right

    if (!setSizeInfo.reactionToSetSize || (childIncreasedWidthBySquashing && setSizeInfo.horizontal != Change::Stretching))
      this->setContentSizeInternal(std::max(this->style.getNaturalWidth(), x),
                                   std::max(this->style.getNaturalHeight(), targetContentHeight));

    // 2 possible things could cause it
    // a) This is reaction to set size, and the size don't match the natural layout
    // b) Call to setContentSizeInternal didn't set the width to x, which can be caused by minimal/maximal width, dontDecrease
    //    width functionality or other things we might add in the future
    // In either a) or b) we need to decide how to solve it by either stretching or squashing the contents
    if (x != this->getContentWidth())
    {
      bool stretch = x < this->getContentWidth() && horizontallyStretchableCount > 0;
      bool squash = x > this->getContentWidth() &&
                   (horizontallyStretchableCount > 0 && extraHorizontalSpaceTakenByHorizontallyStretchable > 0 ||
                    horizontallySquashableCount > 0 ||
                    this->getChildCount() == 1);
      if (stretch || squash)
      {
        int theDiff = (this->getContentWidth() - x);
        x = 0;
        bool widgetIncreasedHeightWhileChangingWidth = false;
        int increasedHeight = 0;
        bool widgetDecreasedheightWhileChangingWidth = false;

        if (squash)
        {
          SquashCalculator targetSizes(GuiDirection::Horizontal);
          int neededSize = this->getContentWidth();
          for (Widget* widget : VisibleChildren(this))
          {
            if (!targetSizes.items.empty())
              neededSize -= this->style.getHorizontalSpacing();
            targetSizes.add(*widget);
          }
          targetSizes.calculate(neededSize);
          for (SquashCalculator::Item& item : targetSizes.items)
          {
            if (item.widget.getWidth() != item.resultSize)
            {
              int originalWidgetHeight = item.widget.getHeight();
              item.widget.setSizeForce(item.resultSize, item.widget.getHeight(), SetSizeInfo(Change::Squashing, Change::Nothing));
              if (originalWidgetHeight < item.widget.getHeight())
              {
                widgetIncreasedHeightWhileChangingWidth = true;
                increasedHeight = Math::max(increasedHeight, item.widget.getHeight());
              }
            }
          }
        }
        else // stretch
        {
           int stretchSize = theDiff / horizontallyStretchableCount;

          // re-position the widgets after the stretching/squashing
          for (Widget* widget : VisibleChildren(this))
          {
            // We are either stretching, or squashing and the un-stretching wasn't enough to get to the target width
            if (stretchSize != 0 && widget->isHorizontallyStretchable())
            {
              int heightBefore = widget->getHeight();
              widget->setSize(widget->getWidth() + stretchSize, widget->getHeight(),
                              SetSizeInfo(stretch ? Change::Stretching : Change::Squashing, Change::Nothing));
              if (heightBefore < widget->getHeight())
              {
                widgetIncreasedHeightWhileChangingWidth = true;
                increasedHeight = Math::max(increasedHeight, widget->getHeight());
              }
              else if (heightBefore > widget->getHeight())
                widgetDecreasedheightWhileChangingWidth = true;
            }
          }
        }

        // re-position the widgets after the stretching/squashing
        for (Widget* widget : VisibleChildren(this))
        {
          // We are either stretching, or squashing and the un-stretching wasn't enough to get to the target width
          x += widget->getLeftMargin();
          widget->setLocation(x, widget->getLocation().y);
          x += widget->getWidth() + this->style.getHorizontalSpacing();
          x += widget->getRightMargin();
        }
        x -= this->style.getHorizontalSpacing(); // compensate for the last item not having horizontal spacing on the right

        // If widget changed height while changing width (typically labels), it might even change the target height of this
        // whole layout which means that widgets need to re-positioned (vertically re-centered for example)
        if (widgetIncreasedHeightWhileChangingWidth || widgetDecreasedheightWhileChangingWidth)
        {
          targetContentHeight = increasedHeight;
          for (Widget* widget : VisibleChildren(this))
            if (setSizeInfo.vertical == Change::Nothing)
              targetContentHeight = std::max(targetContentHeight, Math::max(std::min(widget->getSizeBeforeStretching().height, widget->getHeight()) + widget->getVerticalMargins(), this->style.getNaturalHeight()));
            else
              targetContentHeight = std::max(targetContentHeight, widget->getHeight() + widget->getVerticalMargins());
          targetContentHeight = std::max(targetContentHeight, this->style.getMinimalHeight());
          if (this->style.getMaximalHeight() != 0)
            targetContentHeight = std::min(targetContentHeight, this->style.getMaximalHeight() - this->getVerticalPaddings());
          x = 0;
          for (Widget* widget : VisibleChildren(this))
          {
            int y = 0;
            if (widget->isVerticallyStretchable())
            {
              widget->setSizeForce(widget->getWidth(), targetContentHeight - widget->getVerticalMargins());
              y = widget->getTopMargin();
            }
            else if (verticalAlign == VerticalAlign::Bottom)
              y = targetContentHeight - widget->getHeight() - widget->getBottomMargin();
            else if (verticalAlign == VerticalAlign::Center)
              y = (targetContentHeight - widget->getHeight() - widget->getVerticalMargins()) / 2 + widget->getTopMargin();

            x += widget->getLeftMargin();
            widget->setLocation(x, y);
            x += widget->getWidth() + this->style.getHorizontalSpacing();
            x += widget->getRightMargin();
          }
          if (x != 0)
            x -= this->style.getHorizontalSpacing(); // compensate for the last item not having horizontal spacing on the right
          this->setContentSizeInternal(this->getContentWidth(), targetContentHeight);
        }
        if (x < this->getContentWidth() && this->canShrink(setSizeInfo.reactionToSetSize) ||
            x > this->getContentWidth())
          this->setContentSizeInternal(x, this->getContentHeight());
      }
      else if (horizontalAlign != HorizontalAlign::Left)
      {
        int shift = (this->getContentWidth() - x);
        if (horizontalAlign == HorizontalAlign::Center)
          shift /= 2;
        for (Widget* widget : VisibleChildren(this))
          widget->setLocation(widget->getLocation().x + shift, widget->getLocation().y);
      }
    }
  }

  int HorizontalFlow::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;
    int maxHeight = 0;
    for (const Widget* child : ConstVisibleChildren(this))
      maxHeight = std::max(maxHeight, child->getHeight() + child->getVerticalMargins() - child->maximumVerticalSquashSize());
    maxHeight = std::max(maxHeight, this->style.getMinimalHeight() - this->getVerticalPaddings());
    return std::max(0, this->getContentHeight() - maxHeight);
  }

  int HorizontalFlow::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;
    int result = 0;
    for (const Widget* child : ConstVisibleChildren(this))
      result += child->maximumHorizontalSquashSize();
    int contentWidth = this->getContentWidth();
    int lastWidgetRight = 0;
    if (const Widget* lastVisibleChild = this->visibleBack())
      lastWidgetRight = lastVisibleChild->getLocation().x + lastVisibleChild->getWidth() + lastVisibleChild->getRightMargin();
    if (lastWidgetRight < contentWidth)
      result += contentWidth - lastWidgetRight;
    if (!this->empty())
      result += std::max(0, this->getChildren()[0]->getLocation().x - this->getChildren()[0]->getLeftMargin());
    return std::min(result, this->getWidth() - this->style.getMinimalWidth());
  }

  agui::HorizontalFlow& HorizontalFlow::operator<<(const Pusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->style.setHorizontallyStretchable();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  agui::HorizontalFlow& HorizontalFlow::centerVertically()
  {
    this->style.setVerticalAlign(VerticalAlign::Center);
    return *this;
  }
}

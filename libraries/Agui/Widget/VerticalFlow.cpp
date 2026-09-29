#include <Agui/Widget/EmptyWidget.hpp>
#include <Agui/Widget/VerticalFlow.hpp>
#include <Agui/SquashCalculator.hpp>
#include <algorithm>

namespace agui
{

  int VerticalFlow::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;
    int result = 0;
    for (const Widget* child : ConstVisibleChildren(this))
      result += child->maximumVerticalSquashSize();
    int lastWidgetBottom = 0;
    if (const Widget* lastVisibleChild = this->visibleBack())
      lastWidgetBottom = lastVisibleChild->getLocation().y + lastVisibleChild->getHeight() + lastVisibleChild->getBottomMargin();
    int contentHeight = this->getContentHeight();
    if (lastWidgetBottom < contentHeight)
      result += contentHeight - lastWidgetBottom;
    if (!this->empty())
      result += std::max(0, this->getChildren()[0]->getLocation().y - this->getChildren()[0]->getTopMargin());
    return std::min(result, this->getHeight() - this->style.getMinimalHeight());
  }

  int VerticalFlow::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;
    int maxWidth = 0;
    for (const Widget* child : ConstVisibleChildren(this))
      maxWidth = std::max(maxWidth, child->getWidth() + child->getHorizontalMargins() - child->maximumHorizontalSquashSize());
    maxWidth = std::max(maxWidth, this->style.getMinimalWidth() - this->getHorizontalPaddings());
    return std::max(0, this->getContentWidth() - maxWidth);
  }

  agui::VerticalFlow& VerticalFlow::operator<<(const Pusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->style.setVerticallyStretchable();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  agui::VerticalFlow& VerticalFlow::centerVertically()
  {
    this->style.setVerticalAlign(VerticalAlign::Center);
    return *this;
  }

  VerticalFlowStyle VerticalFlow::defaultStyle;

  VerticalFlow::VerticalFlow(const VerticalFlowStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &VerticalFlow::defaultStyle)
  {
    this->shrinkInReactionToSetSize();
  }

  VerticalFlow::VerticalFlow(HorizontalAlign align)
    : VerticalFlow()
  {
    this->style.setHorizontalAlign(align);
  }

  void VerticalFlow::layoutChildren(SetSizeInfo setSizeInfo)
  {
    if (this->empty())
    {
      if (!setSizeInfo.reactionToSetSize)
        this->setContentSizeInternal(this->style.getNaturalWidth(), this->style.getNaturalHeight());
      return;
    }

    int y = 0;
    int verticallyStretchableCount = 0;
    int verticallySquashableCount = 0;
    int horizontallyStretchableCount = 0;
    int extraVerticalSpaceTakenByVerticallyStretchable = 0;
    int targetContentWidth = 0;
    HorizontalAlign horizontalAlign = this->style.getHorizontalAlign();
    VerticalAlign verticalAlign = this->style.getVerticalAlign();
    bool childChangedHeightByStretching = false;
    bool childDecreasedWithMoreThenRequested = false;
    int childDecreasedWidthMoreThenRequestedValue = 0;

    if (!setSizeInfo.reactionToSetSize)
    {
      for (const Widget* widget : ConstVisibleChildren(this))
        targetContentWidth = std::max(targetContentWidth, widget->getWidth() + widget->getHorizontalMargins());
      targetContentWidth = std::max(targetContentWidth,
                                    std::max(this->style.getMinimalWidth(),
                                             this->style.getNaturalWidth()) - this->getHorizontalPaddings());
      if (this->style.getMaximalWidth() != 0)
        targetContentWidth = std::min(targetContentWidth, this->style.getMaximalWidth() - this->getHorizontalPaddings());
      if (!this->canDecreaseWidth())
        targetContentWidth = std::max(targetContentWidth, this->getContentWidth());
    }
    else
      targetContentWidth = this->getContentWidth();

    for (Widget* widget : VisibleChildren(this))
    {
      int x = 0;
      int widgetHorizontalMargins = widget->getHorizontalMargins();
      if (widget->isHorizontallyStretchable())
      {
        ++horizontallyStretchableCount;
        if (widget->getWidth() - widgetHorizontalMargins < targetContentWidth)
        {
          int heightBefore = widget->getHeight();
          widget->setSize(targetContentWidth - widgetHorizontalMargins, widget->getHeight(), SetSizeInfo(Change::Stretching, Change::Nothing));
          if (widget->getHeight() != heightBefore)
            childChangedHeightByStretching = true;
        }
      }
      if (widget->getWidth() - widgetHorizontalMargins > targetContentWidth)
      {
        int heightBefore = widget->getHeight();
        int targetWidth = targetContentWidth - widgetHorizontalMargins;
        widget->setSize(targetWidth, widget->getHeight(), SetSizeInfo(Change::Squashing, Change::Nothing));
        if (widget->getHeight() != heightBefore)
          childChangedHeightByStretching = true;
        if (widget->getWidth() < targetWidth)
        {
          if (!childDecreasedWithMoreThenRequested)
          {
            childDecreasedWithMoreThenRequested = true;
            childDecreasedWidthMoreThenRequestedValue = widget->getWidth();
          }
          else
            childDecreasedWidthMoreThenRequestedValue = std::min(childDecreasedWidthMoreThenRequestedValue, widget->getWidth());
        }
      }

      if (horizontalAlign == HorizontalAlign::Left)
        x = widget->getLeftMargin();
      else if (horizontalAlign == HorizontalAlign::Right)
        x = targetContentWidth - widget->getWidth() - widget->getRightMargin();
      else // center
        x = (targetContentWidth - widget->getWidth() - widgetHorizontalMargins) / 2 + widget->getLeftMargin();
      y += widget->getTopMargin();
      widget->setLocation(x, y);
      if (widget->isVerticallyStretchable())
      {
        ++verticallyStretchableCount;
        extraVerticalSpaceTakenByVerticallyStretchable += widget->getSize().height - widget->getSizeBeforeStretching().height;
      }
      verticallySquashableCount += widget->isVerticallySquashable();
      y += widget->getHeight() + this->style.getVerticalSpacing();
      y += widget->getBottomMargin();
    }

    y -= this->style.getVerticalSpacing(); // compensate for the last item not having vertical spacing on the bottom

    int modifiedTargetContentWidth = childDecreasedWidthMoreThenRequestedValue;

    if (childDecreasedWithMoreThenRequested)
    {
      if (this->canShrink(setSizeInfo.reactionToSetSize))
      {
        for (Widget* widget : VisibleChildren(this))
        {
          if (this->squashToWidestShrinkingWidget)
            modifiedTargetContentWidth = std::max(modifiedTargetContentWidth, std::min(widget->getSizeBeforeStretching().width, widget->getWidth()) + widget->getHorizontalMargins());
          else
            modifiedTargetContentWidth = std::max(modifiedTargetContentWidth, widget->getWidth() + widget->getHorizontalMargins());
        }
        modifiedTargetContentWidth = std::max(modifiedTargetContentWidth,
                                      std::max(this->style.getMinimalWidth(),
                                               this->style.getNaturalWidth()) - this->getHorizontalPaddings());
        if (this->style.getMaximalWidth() != 0)
          modifiedTargetContentWidth = std::min(modifiedTargetContentWidth, this->style.getMaximalWidth() - this->getHorizontalPaddings());
      }
      else
        modifiedTargetContentWidth = this->getContentWidth();

      // Some of the children might have been squashed, which might result into them avoiding the empty gap when word-wrapping,
      // by making themselves even little bit smaller.

      // This mechanism should still work, and is there to avoid the weird vertical empty gap in dynamically sized windows (tooltips)
      // that have text and are squashed to maximum size.

      // The problem is, when there is more of these elements and all of them are made smaller a bit this way, but the elements are
      // actually horizontally stretchable, so after all the alterations of removing the empty gap, they should still be aligned to the
      // biggest one.
      for (Widget* widget : VisibleChildren(this))
        if (widget->getWidth() != modifiedTargetContentWidth && widget->isHorizontallyStretchable())
          widget->setSize(modifiedTargetContentWidth - widget->getHorizontalMargins(), widget->getHeight());
    }
    else
      modifiedTargetContentWidth = targetContentWidth;

    if (!setSizeInfo.reactionToSetSize)
      this->setContentSizeInternal(std::max(this->style.getNaturalWidth(), modifiedTargetContentWidth),
                                   std::max(this->style.getNaturalHeight(), y));
    else // this is here to solve the case, when flow contains text that gets vertically shorter by stretching, this flow
         // needs to get shorter as well.
      if (modifiedTargetContentWidth != targetContentWidth || childChangedHeightByStretching)  // the condition is here to not break the vertical stretching.
      {
        int widthToSet = 0;
        if (modifiedTargetContentWidth < targetContentWidth && this->canShrink(setSizeInfo.reactionToSetSize))
        {
          for (Widget* widget : VisibleChildren(this))
            if (widget->getWidth() > modifiedTargetContentWidth)
            {
              SetSizeInfo modifiedSetSizeInfo;
              modifiedSetSizeInfo.horizontal = Change::Squashing;
              modifiedSetSizeInfo.vertical = setSizeInfo.vertical;
              widget->setSize(modifiedTargetContentWidth - widget->getHorizontalMargins(), widget->getHeight(), modifiedSetSizeInfo);
            }
          widthToSet = modifiedTargetContentWidth;
        }
        else
         widthToSet = std::max(this->style.getNaturalWidth(), this->getContentWidth());

        int desiredHeight = std::max(this->style.getNaturalHeight(),
                                     (this->isVerticallyStretchable() && setSizeInfo.reactionToSetSize && setSizeInfo.vertical == Change::Stretching) ? this->getContentHeight() : y);
        this->setContentSizeInternal(widthToSet, desiredHeight);
      }

    if (y != this->getContentHeight())
    {
      bool stretch = y < this->getContentHeight() && verticallyStretchableCount > 0;
      bool squash = y > this->getContentHeight() &&
                   (verticallyStretchableCount > 0 && extraVerticalSpaceTakenByVerticallyStretchable > 0 ||
                    verticallySquashableCount > 0 ||
                    this->getChildCount() == 1);
      if (stretch || squash)
      {
        int theDiff = (this->getContentHeight() - y);
        bool widgetIncreasedWidthWhileChangingHeight = false;
        int theWidth = 0;
        if (squash)
        {
          SquashCalculator targetSizes(GuiDirection::Vertical);
          int neededSize = this->getContentHeight();
          for (Widget* widget : VisibleChildren(this))
          {
            if (!targetSizes.items.empty())
              neededSize -= this->style.getVerticalSpacing();
            targetSizes.add(*widget);
          }
          targetSizes.calculate(neededSize);
          for (SquashCalculator::Item& item : targetSizes.items)
          {
            if (item.widget.getHeight() != item.resultSize)
            {
              int originalWidgetWidth = item.widget.getWidth() + item.widget.getHorizontalMargins();
              SetSizeInfo setSizeInfoParameter(setSizeInfo.horizontal, Change::Squashing);
              if (setSizeInfo.reactionToSetSize && setSizeInfo.horizontal == Change::Squashing)
                setSizeInfoParameter.maximalWidth = targetContentWidth;
              int originalHorizontalMargins = item.widget.getHorizontalMargins();

              item.widget.setSizeForce(item.widget.getWidth(), item.resultSize, setSizeInfoParameter);
              int horizontalMargins = item.widget.getHorizontalMargins();
              if (originalWidgetWidth < item.widget.getWidth() + horizontalMargins)
              {
                if (originalHorizontalMargins < horizontalMargins &&
                    item.widget.getWidth() + horizontalMargins > targetContentWidth &&
                    item.widget.getSizeBeforeStretching().width + horizontalMargins <= targetContentWidth)
                  item.widget.setSizeForce(targetContentWidth - horizontalMargins, item.resultSize, setSizeInfoParameter);
                else
                {
                  widgetIncreasedWidthWhileChangingHeight = true;
                  theWidth = std::max(theWidth, item.widget.getWidth() + horizontalMargins);
                }
              }
            }
          }
        }
        else // stretch
        {
          int stretchSize = theDiff / double(stretch ? verticallyStretchableCount : verticallySquashableCount);

          for (Widget* widget : VisibleChildren(this))
          {
            int originalWidgetWidth = widget->getWidth();
            if (stretchSize != 0 && widget->isVerticallyStretchable())
            {
              SetSizeInfo setSizeInfoParameter(setSizeInfo.horizontal, Change::Stretching);
              if (setSizeInfo.reactionToSetSize && setSizeInfo.horizontal == Change::Squashing)
                setSizeInfoParameter.maximalWidth = targetContentWidth;
              widget->setSizeForce(widget->getWidth(), widget->getHeight() + stretchSize, setSizeInfoParameter);
            }
            if (originalWidgetWidth < widget->getWidth())
            {
              widgetIncreasedWidthWhileChangingHeight = true;
              theWidth = std::max(theWidth, widget->getWidth() + widget->getHorizontalMargins());
            }
          }
        }

        y = 0;
        for (Widget* widget : VisibleChildren(this))
        {
          y += widget->getTopMargin();

          int x = 0;
          if (horizontalAlign == HorizontalAlign::Left)
            x = widget->getLeftMargin();
          else if (horizontalAlign == HorizontalAlign::Right)
            x = targetContentWidth - widget->getWidth() - widget->getRightMargin();
          else // center
            x = (targetContentWidth - widget->getWidth()) / 2 + widget->getLeftMargin();

          widget->setLocation(x, y);
          y += widget->getHeight() + this->style.getVerticalSpacing();
          y += widget->getBottomMargin();
        }
        if (y != 0)
          y -= this->style.getVerticalSpacing(); // Compensate for the last item not having the vertical spacing on the bottom

        if (y > this->getHeight())
          this->setContentSizeInternal(this->getWidth(), y); // don't ever let the flow to be squashed more than possible
        if (widgetIncreasedWidthWhileChangingHeight && theWidth > this->getWidth())
        {
          if (this->getChildCount() > 1 && horizontallyStretchableCount > 0)
            this->setContentSize(theWidth, this->getHeight());
          else
            this->setContentSizeInternal(theWidth, this->getHeight());
        }
      }
      else if (verticalAlign != VerticalAlign::Top)
      {
        int shift = (this->getContentHeight() - y);
        if (verticalAlign == VerticalAlign::Center)
          shift /= 2;
        for (Widget* widget : VisibleChildren(this))
          widget->setLocation(widget->getLocation().x, widget->getLocation().y + shift);
      }
    }
  }
}

#include <Agui/SquashCalculator.hpp>
#include <Agui/Widget.hpp>
#include <Agui/Math.hpp>
#include <algorithm>

namespace agui
{

  void SquashCalculator::calculate(int neededSizeSum)
  {
    int currentSum = 0;
    for (Item& item: this->items)
      currentSum += item.resultWithMarginSize;

    // first unstretch stretch widgets (priority)
    while (currentSum > neededSizeSum)
    {
      int biggestExtra = std::numeric_limits<int>::max();
      int biggestExtraCount = 0;
      for (Item& item: this->items)
        if (item.resultWithMarginSize > item.sizeBeforeStretchingWithMargin)
        {
          biggestExtra = std::min(biggestExtra, item.resultWithMarginSize - item.sizeBeforeStretchingWithMargin);
          ++biggestExtraCount;
        }

      if (biggestExtraCount == 0)
        break;

      if (biggestExtra * biggestExtraCount <= currentSum - neededSizeSum)
      {
        for (Item& item: this->items)
          if (item.resultWithMarginSize > item.sizeBeforeStretchingWithMargin)
          {
            item.resultSize -= biggestExtra;
            item.resultWithMarginSize -= biggestExtra;
          }
        currentSum -= biggestExtra * biggestExtraCount;
      }
      else
      {
        int cutPerItem = (currentSum - neededSizeSum) / biggestExtraCount;
        int divisionReminder =  currentSum - neededSizeSum - cutPerItem * biggestExtraCount;
        for (Item& item: this->items)
          if (item.resultWithMarginSize > item.sizeBeforeStretchingWithMargin)
          {
            item.resultSize -= cutPerItem;
            item.resultWithMarginSize -= cutPerItem;
            if (divisionReminder > 0)
            {
              --divisionReminder;
              --item.resultSize;
              --item.resultWithMarginSize;
            }
          }
        return;
      }
    }

    // when unstretching wasn't enough, lets squash
    while (currentSum > neededSizeSum)
    {
      int biggestExtra = std::numeric_limits<int>::max();
      int biggestExtraCount = 0;
      for (Item& item: this->items)
        if (item.resultWithMarginSize > item.minimumWithMarginSize)
        {
          biggestExtra = std::min(biggestExtra, item.resultWithMarginSize - item.minimumWithMarginSize);
          ++biggestExtraCount;
        }

      if (biggestExtraCount == 0)
        return;

      if (biggestExtra * biggestExtraCount <= currentSum - neededSizeSum)
      {
        for (Item& item: this->items)
          if (item.resultWithMarginSize > item.minimumWithMarginSize)
          {
            item.resultSize -= biggestExtra;
            item.resultWithMarginSize -= biggestExtra;
          }
        currentSum -= biggestExtra * biggestExtraCount;
      }
      else
      {
        int cutPerItem = (currentSum - neededSizeSum) / biggestExtraCount;
        int divisionReminder =  currentSum - neededSizeSum - cutPerItem * biggestExtraCount;
        for (Item& item: this->items)
          if (item.resultWithMarginSize > item.minimumWithMarginSize)
          {
            item.resultSize -= cutPerItem;
            item.resultWithMarginSize -= cutPerItem;
            if (divisionReminder > 0)
            {
              --divisionReminder;
              --item.resultSize;
              --item.resultWithMarginSize;
            }
          }
        return;
      }
    }
  }

  void SquashCalculator::add(Widget& widget)
  {
    this->items.emplace_back(this->direction, widget);
  }

  void SquashCalculator::merge(Widget& widget)
  {
    this->items.back().merge(Item(this->direction, widget));
  }

  void SquashCalculator::merge(uint32_t index, Widget& widget)
  {
    this->items[index].merge(Item(this->direction, widget));
  }

  SquashCalculator::Item::Item(GuiDirection direction, Widget& widget)
    : widget(widget)
    , minimumSize(direction == GuiDirection::Horizontal ?
                    (widget.getWidth() - widget.maximumHorizontalSquashSize()) :
                    (widget.getHeight() - widget.maximumVerticalSquashSize()))
    , minimumWithMarginSize(this->minimumSize + (direction == GuiDirection::Horizontal ? widget.getHorizontalMargins() : widget.getVerticalMargins()))
    , sizeBeforeStretching(Math::max(direction == GuiDirection::Horizontal ? widget.getSizeBeforeStretching().width : widget.getSizeBeforeStretching().height, this->minimumSize))
    , sizeBeforeStretchingWithMargin(Math::max(this->sizeBeforeStretching + (direction == GuiDirection::Horizontal ? widget.getHorizontalMargins() : widget.getVerticalMargins()), this->minimumWithMarginSize))
    , resultSize(Math::max(direction == GuiDirection::Horizontal ? widget.getWidth() : widget.getHeight(), this->minimumSize))
    , resultWithMarginSize(Math::max(this->resultSize + (direction == GuiDirection::Horizontal ? widget.getHorizontalMargins() : widget.getVerticalMargins()), this->minimumWithMarginSize))
{}

  void SquashCalculator::Item::merge(const Item& other)
  {
    this->minimumSize = Math::max(this->minimumSize, other.minimumSize);
    this->minimumWithMarginSize = Math::max(this->minimumWithMarginSize, other.minimumWithMarginSize);
    this->sizeBeforeStretching = Math::max(this->sizeBeforeStretching, other.sizeBeforeStretching);
    this->sizeBeforeStretchingWithMargin = Math::max(this->sizeBeforeStretchingWithMargin, other.sizeBeforeStretchingWithMargin);
    this->resultSize = Math::max(this->resultSize, other.resultSize);
    this->resultWithMarginSize = Math::max(this->resultWithMarginSize, other.resultWithMarginSize);
  }

  void SquashCalculator::Item::ensureMinimum(int minimum)
  {
    this->minimumSize = Math::max(this->minimumSize, minimum);
    this->minimumWithMarginSize = Math::max(this->minimumWithMarginSize, minimum);
    this->sizeBeforeStretching = Math::max(this->sizeBeforeStretching, minimum);
    this->sizeBeforeStretchingWithMargin = Math::max(this->sizeBeforeStretchingWithMargin, minimum);
    this->resultSize = Math::max(this->resultSize, minimum);
    this->resultWithMarginSize = Math::max(this->resultWithMarginSize, minimum);
  }

}

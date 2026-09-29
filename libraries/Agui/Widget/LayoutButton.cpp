#include <Agui/Widget/LayoutButton.hpp>
#include <Agui/Widget/HorizontalFlow.hpp>
#include <Agui/Widget/VerticalFlow.hpp>
#include <algorithm>

namespace agui
{
  LayoutButton::LayoutButton(GuiDirection guiDirection, const ButtonStyle* parentStyle)
    : Button(parentStyle)
    , direction(guiDirection)
    , layout(direction == GuiDirection::Horizontal ?
             static_cast<agui::Layout*>(new agui::HorizontalFlow()) :
             static_cast<agui::Layout*>(new agui::VerticalFlow()))
  {
    this->layout->autoDestructWhenRemovedFromParent();
    this->addPrivateChild(this->layout);
  }

  bool LayoutButton::empty() const
  {
    return this->layout->empty();
  }

  void LayoutButton::add(Widget* widget)
  {
    this->layout->add(widget);
  }

  void LayoutButton::insert(Widget* widget, uint32_t index)
  {
    this->layout->insert(widget, index);
  }

  void LayoutButton::addFront(Widget* widget)
  {
    this->layout->addFront(widget);
  }

  void LayoutButton::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
     if (this->layout->containsChildWidget(widget))
      this->layout->remove(widget, keepWidgetAlive);
    else
      Widget::remove(widget, keepWidgetAlive);
  }

  void LayoutButton::clear()
  {
    this->layout->clear();
  }

  void LayoutButton::setSize(int width, int height, SetSizeInfo setSizeInfo /*= SetSizeInfo()*/)
  {
    super::setSize(width, height, setSizeInfo);
    int oldContentHeight = this->getContentSize().height;
    int oldContentWidth = this->getContentSize().width;
    this->layout->setSize(oldContentWidth, oldContentHeight, setSizeInfo);

    if (oldContentHeight > layout->getHeight() &&
        (this->canShrink(setSizeInfo.reactionToSetSize) || setSizeInfo.vertical == Change::Nothing))  // content is smaller than required, make this also smaller, but not smaller than natural height
      super::setSize(this->getWidth(), std::max(layout->getHeight() + this->getVerticalPaddings(), this->style.getNaturalHeight()), setSizeInfo);
    else if (oldContentHeight < layout->getHeight())
      super::setSize(this->getWidth(), layout->getHeight() + this->getVerticalPaddings(), setSizeInfo); // result size is bigger, just enlarge this

    if (oldContentWidth > layout->getWidth() && this->canShrink(setSizeInfo.reactionToSetSize)) // content is smaller than required, make this also smaller, but not smaller than natural width
      super::setSize(std::max(layout->getWidth() + this->getHorizontalPaddings(), this->style.getNaturalWidth()), this->getHeight(), setSizeInfo);
    else if (oldContentWidth < layout->getWidth())
      super::setSize(layout->getWidth() + this->getHorizontalPaddings(), this->getHeight(), setSizeInfo); // result width is bigger, just enlarge this
  }

  void LayoutButton::changeClickState(Clickable::ClickState state)
  {
    // Skip calling Button::changeClickState because it assumes the button contains a Label.
    this->Clickable::changeClickState(state);
  }

  void LayoutButton::resizeToContents()
  {
    if (this->layout->isVisible())
      this->setContentSizeInternal(std::max(this->layout->getWidth(), this->style.getNaturalWidth() - this->getHorizontalPaddings()),
                                   std::max(this->layout->getHeight(), this->style.getNaturalHeight() - this->getVerticalPaddings()));
    else
      this->setContentSizeInternal(this->style.getNaturalWidth() - this->getHorizontalPaddings(),
                                   this->style.getNaturalHeight() - this->getVerticalPaddings());
    Dimension contentSize = this->getContentSize();

    // limited by the style max width/height
    if (contentSize != this->layout->getSize() && this->layout->isVisible())
    {
      int originalWidth = this->layout->getWidth();
      int originalHeight = this->layout->getHeight();
      SetSizeInfo setSizeInfo(Change::Nothing, Change::Nothing);
      int targetWidth = this->layout->getWidth();
      int targetHeight = this->layout->getHeight();

      if (contentSize.width > this->layout->getWidth())
      {
        if (this->layout->isHorizontallyStretchable())
        {
          setSizeInfo.horizontal = Change::Stretching;
          targetWidth = contentSize.width;
        }
      }
      else if (contentSize.width < this->layout->getWidth())
      {
        setSizeInfo.horizontal = Change::Squashing;
        targetWidth = contentSize.width;
      }

      if (contentSize.height > this->layout->getHeight())
      {
        if (this->layout->isVerticallyStretchable())
        {
          setSizeInfo.vertical = Change::Stretching;
          targetHeight = contentSize.height;
        }
      }
      else if (contentSize.height < this->layout->getHeight())
      {
        setSizeInfo.vertical = Change::Squashing;
        targetHeight = contentSize.height;
      }
      if (this->layout->getSize() != Dimension(targetWidth, targetHeight))
        this->layout->setSize(targetWidth, targetHeight, setSizeInfo);

      if (this->layout->getWidth() != originalWidth ||
          this->layout->getHeight() != originalHeight)
      {
        int desiredWidth = std::max(this->layout->getWidth(), this->style.getNaturalWidth() - this->getHorizontalPaddings());
        int desiredHeight = std::max(this->layout->getHeight(), this->style.getNaturalHeight() - this->getVerticalPaddings());
        this->setContentSizeInternal(desiredWidth, desiredHeight);
      }
      int x = 0, y = 0;

      agui::HorizontalAlign horizontalAlign = this->style.getHorizontalAlign();
      if (horizontalAlign != agui::HorizontalAlign::Left)
      {
        int extraHorizontalSize = this->getContentSize().width - this->layout->getWidth();
        if (extraHorizontalSize > 0)
          if (horizontalAlign == agui::HorizontalAlign::Right)
            x = this->getContentSize().width - this->layout->getWidth();
          else if (horizontalAlign == agui::HorizontalAlign::Center)
            x = (this->getContentSize().width - this->layout->getWidth()) / 2;
      }

      agui::VerticalAlign verticalAlign = this->style.getVerticalAlign();
      if (verticalAlign != agui::VerticalAlign::Top)
      {
        int extraVerticalSize = this->getContentSize().height - this->layout->getHeight() - y;
        if (extraVerticalSize > 0)
          if (verticalAlign == agui::VerticalAlign::Bottom)
            y = this->getContentSize().height - this->layout->getHeight();
          else if (verticalAlign == agui::VerticalAlign::Center)
            y = (this->getContentSize().height - this->layout->getHeight()) / 2;
      }
      if (this->layout->getLocation() != agui::Point(x, y))
        this->layout->setLocation(x, y);
    }
  }

  agui::VerticalFlow* LayoutButton::getVerticalFlow()
  {
    return this->direction == GuiDirection::Vertical ? static_cast<VerticalFlow*>(this->layout) : nullptr;
  }

  agui::HorizontalFlow* LayoutButton::getHorizontalFlow()
  {
    return this->direction == GuiDirection::Horizontal ? static_cast<HorizontalFlow*>(this->layout) : nullptr;
  }
}

#include "Agui/Widget/ScrollBar.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Image.hpp"
#include <algorithm>
#include <Agui/EventDispatchHelper.hpp>

namespace agui
{
  ScrollBarStyle HorizontalPolicy::defaultStyle;

  void HorizontalPolicy::setSize(Button& thumb, int32_t size, int32_t minimalSize)
  {
    const ElementImageSet* imageSet = thumb.style.getDefaultGraphicalSet();
    if (imageSet->base.left &&
        imageSet->base.right &&
        imageSet->base.center &&
        imageSet->base.tilingFlags & ElementImageSet::Layer::CENTER_TILING_HORIZONTAL)
    {
      int horizontalBorders = (imageSet->base.left->getWidth() + imageSet->base.right->getWidth()) * Gui::instance->scale;
      size -= (size - horizontalBorders) % int(imageSet->base.center->getWidth() * Gui::instance->scale);
      while (size < minimalSize * Gui::instance->scale)
        size += imageSet->base.center->getWidth() * Gui::instance->scale;
    }
    thumb.setSize(size, thumb.getHeight());
  }

  void HorizontalPolicy::setLocation(Widget& thumb, int location, const Dimension holderSize)
  {
    thumb.setLocation(location, (holderSize.height - thumb.getHeight()) / 2);
  }

  ScrollBarStyle VerticalPolicy::defaultStyle;

  void VerticalPolicy::setSize(Button& thumb, int32_t size, int32_t minimalSize)
  {
    const ElementImageSet* imageSet = thumb.style.getDefaultGraphicalSet();
    if (imageSet->base.top &&
        imageSet->base.bottom &&
        imageSet->base.center &&
        imageSet->base.tilingFlags & ElementImageSet::Layer::CENTER_TILING_VERTICAL)
    {
      int verticalBorders = (imageSet->base.top->getHeight() + imageSet->base.bottom->getHeight()) * Gui::instance->scale;
      size -= (size - verticalBorders) % int(imageSet->base.center->getHeight() * Gui::instance->scale);
      while (size < minimalSize * Gui::instance->scale)
        size += imageSet->base.center->getHeight() * Gui::instance->scale;
    }
    thumb.setSize(thumb.getWidth(), size);
  }

  void VerticalPolicy::setLocation(Widget& thumb, int location, const Dimension holderSize)
  {
    thumb.setLocation((holderSize.width - thumb.getWidth()) / 2, location);
  }

  template<typename Policy>
  ScrollBar<Policy>::ScrollBar(const ScrollBarStyle* parentStyle)
    : style(this, parentStyle)
    , thumb(this->style.getThumbButtonStyle())
  {
    this->thumb.setMouseLeaveState(TextButton::ClickState::CLICKED);
    this->thumb.setFocusable(false);
    this->thumb.onMouseDown(this,
      [this](const agui::MouseEvent& mouseEvent)
      {
        this->downThumbPos = Policy::getCoordinate(this->thumb.getLocation());
        this->downMousePos = Policy::getCoordinate(mouseEvent.getPosition()) + Policy::getCoordinate(this->thumb.getLocation());
      });
    this->thumb.onMouseDrag(this,
      [this](const agui::MouseEvent& mouseEvent)
      {
        if (!this->reactToThumbDrag)
          return;
        int mouseChange = Policy::getCoordinate(mouseEvent.getPosition()) + Policy::getCoordinate(this->thumb.getLocation()) - this->downMousePos;
        int thumbChange = this->downThumbPos + mouseChange;
        this->setValue(this->getValueFromPosition(thumbChange));
        this->dispatchSliderMove(this->getValue());
      });
    this->thumb.onMouseWheelDown(this, [this](const agui::MouseEvent& mouseEvent) { this->wheelScrollDown(mouseEvent.getMouseWheelChange()); });
    this->thumb.onMouseWheelUp(this, [this](const agui::MouseEvent& mouseEvent) { this->wheelScrollUp(mouseEvent.getMouseWheelChange()); });

    this->thumb.setKeepSize(true);
    this->addPrivateChild(&this->thumb);
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMaxThumbSize() const
  {
    return Policy::getContentDimension(*this);
  }

  template<typename Policy>
  int ScrollBar<Policy>::getLargeAmount() const
  {
    return this->largeAmount;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getValue() const
  {
    return this->currentValue;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMinValue() const
  {
    return this->minValue;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMaxValue() const
  {
    return this->maxValue;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMaxUsefulValue() const
  {
    return this->getMaxValue() - this->getLargeAmount();
  }

  template<typename Policy>
  void ScrollBar<Policy>::positionThumb()
  {
    float value = getAdjustedMaxThumbSize() * getRelativeValue();

    if (this->getValue() == this->getMaxValue() - this->getLargeAmount())
      value = (float)(Policy::getContentDimension(*this) -
                      Policy::getDimension(this->thumb));

    if (value + Policy::getDimension(this->thumb) > Policy::getContentDimension(*this))
      value = (float)(Policy::getContentDimension(*this) -
                      Policy::getDimension(this->thumb));
    Policy::setLocation(this->thumb, (int)value, this->getSize());
  }

  template<typename Policy>
  void ScrollBar<Policy>::resizeThumb()
  {
    int size = this->getLargeAmount(); // the size if 1 pixel = 1 value
    int maxValSupport = this->getMaxValue() - this->getMinValue();
    // get the ratio
    float change = (float)this->getMaxThumbSize() / (float)maxValSupport;

    // make size proportional to ratio
    size = (int)((float)size * change);

    // make sure the thumb never gets too small
    if (size < this->getMinThumbSize())
      size = this->getMinThumbSize();

    if (size >= this->getMaxThumbSize())
      this->thumb.setVisibleSilent(false);
    else
      this->thumb.setVisibleSilent(true);

    Policy::setSize(this->thumb, size, this->getMinThumbSize());
  }

  template<typename Policy>
  void ScrollBar<Policy>::setLargeAmount(int amount)
  {
    int maxVal = this->getMaxValue() - this->getMinValue();
    if (amount > maxVal)
      amount = maxVal;
    if (amount < 0)
      amount = 0;

    this->largeAmount = amount;
    this->resizeThumb();
    this->positionThumb();
  }

  template<typename Policy>
  void ScrollBar<Policy>::setValue(int value, bool dispatchChange)
  {
    int targetValue = value;

    if (targetValue <= this->getMinValue())
      targetValue = this->getMinValue();
    else if (targetValue >= this->getMaxValue() - this->largeAmount)
      targetValue = this->getMaxValue() - this->largeAmount;

    if (targetValue != this->currentValue)
    {
      this->currentValue = targetValue;
      this->positionThumb();
      if (dispatchChange)
        this->dispatchSliderMove(this->getValue());
    }
  }

  template<typename Policy>
  void ScrollBar<Policy>::scrollToCenter()
  {
    this->setValue((this->getMaxValue() - this->getLargeAmount()) / 2);
  }

  template<typename Policy>
  void ScrollBar<Policy>::setMinValue(int value)
  {
    if (value <= this->getMaxValue())
    {
      this->minValue = value;
      if (this->getValue() < minValue)
        this->setValue(minValue);
      this->positionThumb();
      this->resizeThumb();
    }
  }

  template<typename Policy>
  void ScrollBar<Policy>::setMaxValue(int value)
  {
    if (value >= this->getMinValue())
    {
      this->maxValue = value;
      if (this->getLargeAmount() >= maxValue)
        this->setLargeAmount(maxValue);
      if (this->getValue() >= this->maxValue - this->largeAmount && this->maxValue - this->largeAmount > 1)
        this->setValue(this->maxValue - this->largeAmount);
      this->positionThumb();
      this->resizeThumb();
    }
  }

  template<typename Policy>
  float ScrollBar<Policy>::getRelativeValue() const
  {
    float relVal = (float)(getValue() - getMinValue());
    float relMax = (float) (getMaxValue() - getMinValue());

    return relVal / relMax;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getValueFromPosition(int position) const
  {
    // what percent of the thumb size we have traveled
    float retVal = ((float)position / (float)getAdjustedMaxThumbSize());

    // total possible number of values
    int numValues = getMaxValue() - getMinValue();

    // how many values we have passed
    retVal = retVal * numValues;

    // add the minimum to get the value
    retVal += (float)getMinValue();

    // bounds checking
    if (retVal > getMaxValue() - getLargeAmount())
      retVal = (float)(getMaxValue() - getLargeAmount());
    if (retVal < getMinValue())
      retVal = (float)getMinValue();

    return (int)retVal;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::isThumbAtEnd() const
  {
    return Policy::getCoordinate(this->thumb.getLocation()) + Policy::getDimension(this->thumb) == this->getMaxThumbSize();
  }

  template<typename Policy>
  bool ScrollBar<Policy>::isThumbAtStart() const
  {
    return Policy::getCoordinate(this->thumb.getLocation()) + Policy::getDimension(this->thumb) == 0;
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMinThumbSize() const
  {
    return this->minThumbSize;
  }

  template<typename Policy>
  void ScrollBar<Policy>::setMinThumbSize(int size)
  {
    if (size >= 0)
      this->minThumbSize = size;
  }

  template<typename Policy>
  void ScrollBar<Policy>::reapplySubStyles()
  {
    this->thumb.style.setParent(this->style.getThumbButtonStyle());
  }

  template<typename Policy>
  void ScrollBar<Policy>::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->resizeThumb();
    this->positionThumb();
  }

  template<typename Policy>
  int ScrollBar<Policy>::getAdjustedMaxThumbSize() const
  {
    // the size if 1 pixel = 1 value
    int size = this->getLargeAmount();

    int maxValSupport = getMaxValue() - getMinValue();
    // get the ratio
    float change = (float)(Policy::getDimension(*this)) / (float)maxValSupport;

    // make size proportional to ratio
    size = (int)((float)size * change);

    int difference = size - Policy::getDimension(this->thumb);

    return Policy::getDimension(*this) + difference;

  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseDoubleClick(const MouseEvent& mouseEvent)
  {
    if (this->getParent())
      return this->getParent()->mouseDoubleClick(mouseEvent.copyWithNewSource(this));
    return false;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseDown(const MouseEvent& mouseEvent)
  {
    // when you click, it scrolls
    int mousePos = Policy::getCoordinate(mouseEvent.getPosition());
    int newVal = getValue();
    if (mousePos > Policy::getCoordinate(this->thumb.getLocation()))
      newVal += getLargeAmount();
    else
      newVal -= getLargeAmount();
    this->setValue(newVal, true);
    return true;
  }

  template<typename Policy>
  void ScrollBar<Policy>::setRangeFromPage(int pageSize, int contentSize)
  {
    if (pageSize <= 0 || contentSize <= 0 || pageSize >= contentSize)
    {
      this->setMaxValue(1);
      this->setLargeAmount(1);
      return;
    }

    // should the thumb be brought to the bottom?
    bool isAtMax = this->getValue() > contentSize - pageSize;
    bool atBottom = this->getValue() == this->getMaxValue() - this->getLargeAmount();

    this->setMaxValue(contentSize);
    this->setLargeAmount(pageSize);

    if (isAtMax || (atBottom && this->isStickingToBottom()))
      this->setValue(this->getMaxValue() - this->getLargeAmount());
  }

  template<typename Policy>
  bool ScrollBar<Policy>::isStickingToBottom() const
  {
    return stickToBottom;
  }

  template<typename Policy>
  void ScrollBar<Policy>::setStickToBottom()
  {
    stickToBottom = true;
  }

  template<typename Policy>
  void ScrollBar<Policy>::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getBackgroundGraphicalSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  template<typename Policy>
  void ScrollBar<Policy>::setMouseWheelAmount(int amount)
  {
    wheelSpeed = amount;
  }

  template<typename Policy>
  void ScrollBar<Policy>::wheelScrollDown(int deltaWheel)
  {
    this->setValue(this->getValue() + this->getMouseWheelAmount() - deltaWheel, true);
  }

  template<typename Policy>
  void ScrollBar<Policy>::wheelScrollUp(int deltaWheel)
  {
    this->setValue(this->getValue() - this->getMouseWheelAmount() - deltaWheel, true);
  }

  template<typename Policy>
  void ScrollBar<Policy>::wheelScrollLeft(int deltaWheel)
  {
    this->setValue(this->getValue() + this->getMouseWheelAmount() - deltaWheel, true);
  }

  template<typename Policy>
  void ScrollBar<Policy>::wheelScrollRight(int deltaWheel)
  {
    this->setValue(this->getValue() - this->getMouseWheelAmount() - deltaWheel, true);
  }

  template<typename Policy>
  int ScrollBar<Policy>::getMouseWheelAmount() const
  {
    return this->wheelSpeed;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseWheelDown(const MouseEvent& mouseEvent)
  {
    this->wheelScrollDown(mouseEvent.getMouseWheelChange());
    return true;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseWheelUp(const MouseEvent& mouseEvent)
  {
    this->wheelScrollUp(mouseEvent.getMouseWheelChange());
    return true;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseWheelLeft(const MouseEvent& mouseEvent)
  {
    this->wheelScrollLeft(mouseEvent.getMouseWheelChange());
    return true;
  }

  template<typename Policy>
  bool ScrollBar<Policy>::mouseWheelRight(const MouseEvent& mouseEvent)
  {
    this->wheelScrollRight(mouseEvent.getMouseWheelChange());
    return true;
  }

  template<typename Policy>
  void ScrollBar<Policy>::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (forceInFrame)
      super::recursivePaintShadows(enabled, graphicsContext, absolutePosition, forceInFrame, includeThis);
  }

  template<typename Policy>
  void ScrollBar<Policy>::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    // The custom logic of recursive paintChildren is here to force the bar shadow be on top of the background
    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, this->getSizeRectangle()))
    {
      this->paint(PaintEvent(enabled, graphicsContext), absolutePosition);
      this->paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition);
      if (this->thumb.isVisible() && this->thumb.shouldRender())
      {
        graphicsContext->setOffset(absolutePosition + this->thumb.getLocation());
        this->thumb.paint(PaintEvent(enabled, graphicsContext), absolutePosition + this->thumb.getLocation());
        graphicsContext->setOffset(absolutePosition + this->thumb.getLocation());
        this->thumb.paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition + this->thumb.getLocation());
        graphicsContext->setOffset(absolutePosition);
      }
    }
    graphicsContext->popClippingRect();
  }

  template<typename Policy>
  void agui::ScrollBar<Policy>::onSliderMove(GenericTargetable* owner, std::function<void(double)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnSliderMove);
    this->actionListeners.back().onSliderMove = std::move(callback);
  }

  template<typename Policy>
  void agui::ScrollBar<Policy>::onSliderMove(GenericTargetable* owner, std::function<void()> callback)
  {
    return this->onSliderMove(owner, [callback = std::move(callback)](double) { callback(); });
  }

  template class ScrollBar<HorizontalPolicy>;
  template class ScrollBar<VerticalPolicy>;
}

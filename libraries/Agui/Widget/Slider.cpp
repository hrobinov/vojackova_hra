#include "Agui/Exception.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/TextButton.hpp"
#include "Agui/Widget/Slider.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace agui
{
  SliderStyle Slider::defaultStyle;

  Slider::Slider(const SliderStyle* parentStyle, ExternalMouseDragProcessing externalMouseDragProcessing)
    : style(this, parentStyle ? parentStyle : &Slider::defaultStyle)
    , marker(this->style.getButtonStyle())
  {
    this->marker.setMouseLeaveState(TextButton::ClickState::CLICKED);
    this->addPrivateChild(&this->marker);
    this->marker.setFocusable(false);
    this->setFocusable(true);
    if (!externalMouseDragProcessing)
      this->marker.onMouseDrag(this,
        [this](const MouseEvent& mouseEvent)
        {
           if (!this->shouldReactToEvent(mouseEvent))
            return;
          double newValue = this->positionToValue(mouseEvent.getPosition().x + mouseEvent.getSourceWidget()->getLocation().x);
          this->setValue(newValue, true);
         });
    this->marker.onMouseEnter(this,
      [this](const MouseEvent& mouseEvent)
      {
        // hovering button if the mouse came from this (the slider) and it already has a tooltip
        // this is to avoid having to wait for the mouse hover delay again when moving between a slider and its button
        if (mouseEvent.getSourceWidget() == &this->marker &&
            mouseEvent.previous == this &&
            this->getToolTip())
          this->marker.mouseHover(mouseEvent);
       });

    this->positionMarker(this->getMinValue());
    this->setValue(this->getMinValue());
  }

  bool Slider::shouldReactToEvent(const MouseEvent& mouseEvent) const
  {
    return mouseEvent.getButton() == MouseButton::LEFT && this->isEnabled();
  }

  void Slider::positionMarker(double value)
  {
    const int center = this->getContentHeight() - this->marker.getHeight();
    this->marker.setLocation(this->valueToPosition(value), center);
  }

  void Slider::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->shadowView)
      this->drawNotchesAndBars(paintEvent, absolutePosition, ElementImageSet::LayerType::Shadow);
  }

  void Slider::drawNotch(const PaintEvent& paintEvent, const agui::Point& absolutePosition, const ElementImageSet* notch, ElementImageSet::LayerType layer, double horizontalPos)
  {
    // this is also relying on the notch being a monolith picture, so it should probably be also a part of style definition instead.
    int notchWidth = notch->base.center ? notch->base.center->getWidth() * Gui::scale : 0;
    int notchHeight = notch->base.center ? notch->base.center->getHeight() * Gui::scale : 0;
    // horizontal pos is the middle of the notch
    notch->getLayer(layer).draw(paintEvent, Rectangle(std::round(horizontalPos - notchWidth / 2.0), 0,
                                                      notchWidth, notchHeight), absolutePosition);
  }

  void Slider::drawNotchesAndBars(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layer)
  {
    int barTop = 0;
    if (this->style.shouldDrawNotches())
    {
      const ElementImageSet* notch = this->style.getNotch();

      int notchesNumber = this->getRange() / this->getValueStep() + 1 + this->getExtraNotchCount();
      double firstPosition = this->marker.getWidth() / 2; // position of the first notch (it's middle)
      double stepSize = (this->getContentWidth() - 2.0 * firstPosition) / double(notchesNumber - 1);
      for (int i = 0; i < notchesNumber; i++)
        this->drawNotch(paintEvent, absolutePosition, notch, layer, firstPosition + i * stepSize);

      // @warning, looking on the center image to figure out the height of the notch is expecting that the element image
      // is a monolith. Better solution would probably be to have the notch height as a separate style value.
      barTop = (notch->base.center ? notch->base.center->getHeight() : 0 ) * Gui::scale; // bar will be drawn under this
    }
    this->drawBars(paintEvent, absolutePosition, layer, barTop);
  }

  const ElementImageSet* Slider::getFullBarElementImageSet() const
  {
    return this->isEnabled() ? this->style.getFullBar() : this->style.getFullBarDisabled();
  }

  int Slider::getMarkerCenter(agui::Button& marker)
  {
    return marker.getLocation().x + marker.getWidth() / 2;
  }

  void Slider::postDisplaySizeChanged()
  {
    super::displaySizeChanged();
    this->positionMarker(this->getValue());
  }

  void Slider::paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->drawNotchesAndBars(paintEvent, absolutePosition, ElementImageSet::LayerType::Base);
  }

  void Slider::drawMarker(agui::Button& marker, const Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    graphicsContext->setOffset(absolutePosition + marker.getLocation());
    marker.paint(PaintEvent(enabled, graphicsContext), absolutePosition + marker.getLocation());
  }

  void Slider::drawMarkerGlowAndShadow(agui::Button& marker, const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    graphicsContext->setOffset(absolutePosition + marker.getLocation());
    marker.paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition);
    graphicsContext->setOffset(absolutePosition + marker.getLocation());
    marker.paintBackgroundGlow(PaintEvent(enabled, graphicsContext), absolutePosition);
  }

  void Slider::drawMarkers(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    this->drawMarker(this->marker, absolutePosition, enabled, graphicsContext);
  }

  void Slider::drawMarkersGlowAndShadow(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    this->drawMarkerGlowAndShadow(this->marker, absolutePosition, enabled, graphicsContext);
  }

  void Slider::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    // The custom logic of recursive paintChildren is here to force the marker shadow be on top of the slider bars.
    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, this->getSizeRectangle()))
    {
      this->paint(PaintEvent(enabled, graphicsContext), absolutePosition);
      this->paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition);
      // in double slider the two marker should not cast glows onto each other
      this->drawMarkersGlowAndShadow(absolutePosition, enabled, graphicsContext);
      this->drawMarkers(absolutePosition, enabled, graphicsContext);
      graphicsContext->setOffset(absolutePosition);
    }
    graphicsContext->popClippingRect();
  }

  void Slider::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (forceInFrame)
      super::recursivePaintShadows(enabled, graphicsContext, absolutePosition, forceInFrame, includeThis);
  }

  void Slider::recursivePaintGlows(bool, Graphics*, const Point&, bool, bool)
  {
    // Glows are handled directly when painting children
  }

  void Slider::drawBars(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layerType, int barTop)
  {
    int markerCenter = this->getMarkerCenter(this->marker);
    // left, full part of the bar
    this->getFullBarElementImageSet()->getLayer(layerType).draw(paintEvent, Rectangle(0, barTop, markerCenter, this->getContentHeight() - barTop), absolutePosition);
    const ElementImageSet* emptyBar = this->isEnabled() ? this->style.getEmptyBar() : this->style.getEmptyBarDisabled();
    barTop += int ((this->getContentHeight() - barTop - emptyBar->base.left->getHeight() * Gui::scale) / 2); // the empty bar can be thinner
    int rightBarHeight = emptyBar->base.left->getHeight();
    emptyBar->getLayer(layerType).draw(paintEvent, Rectangle(markerCenter,
                                                             barTop,
                                                             this->getContentWidth() - markerCenter,
                                                             int(rightBarHeight * Gui::scale)), absolutePosition);
  }

  void Slider::handleKeyboard(const KeyEvent& keyEvent)
  {
    if (!this->isEnabled())
      return;
    if (keyEvent.getExtendedKey() == EXT_KEY_LEFT)
      this->setValue(this->getValue() - this->getValueStep());
    else if (keyEvent.getExtendedKey() == EXT_KEY_RIGHT)
      this->setValue(this->getValue() + this->getValueStep());
  }

  bool Slider::keyDown(const KeyEvent& keyEvent)
  {
    const double oldValue = this->getValue();
    this->handleKeyboard(keyEvent);
    double newValue = this->getValue();
    if (newValue != oldValue)
      this->dispatchSliderMove(newValue);
    return true;
  }

  bool Slider::keyRepeat(const KeyEvent& keyEvent)
  {
    const double oldValue = this->getValue();
    this->handleKeyboard(keyEvent);
    double newValue = this->getValue();
    if (newValue != oldValue)
      this->dispatchSliderMove(newValue);
    return true;
  }

  bool Slider::mouseDown(const MouseEvent& mouseEvent)
  {
    if (!this->shouldReactToEvent(mouseEvent))
      return false;

    double newValue = this->positionToValue(mouseEvent.getPosition().x - this->getContentRectangle().getLeft());
    this->setValue(newValue, true);
    if (this->getGui())
    {
      this->getGui()->startDragging(&this->marker);
      this->marker.mouseEnter(mouseEvent);
      this->removeToolTipWidget();
      this->marker.mouseHover(mouseEvent);
    }
    return true;
  }

  bool Slider::mouseWheelDown(const MouseEvent&)
  {
    if (!this->isEnabled())
      return false;
    double newValue = this->getValue() - this->getValueStep();
    if (this->getValue() != newValue)
      this->setValue(newValue, true);
    return true;
  }

  bool Slider::mouseWheelUp(const MouseEvent&)
  {
    if (!this->isEnabled())
      return false;
    double newValue = this->getValue() + this->getValueStep();

    if (this->getValue() != newValue)
      this->setValue(newValue, true);
    return true;
  }

  bool Slider::mouseEnter(const MouseEvent& mouseEvent)
  {
    super::mouseEnter(mouseEvent);

    // hovering itself if the mouse came from the button that already has a tooltip
    // this is to avoid having to wait for the mouse hover delay again when moving between a slider and its button
    if (mouseEvent.previous == &this->marker &&
        this->marker.getToolTip())
      this->mouseHover(mouseEvent);
    return true;
  }

  void Slider::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->positionMarker(this->lastMarkerValue);
  }

  Widget& Slider::setEnabled(bool value)
  {
    super::setEnabled(value);
    this->marker.setEnabled(value);
    return *this;
  }

  void Slider::reapplySubStyles()
  {
    this->marker.style.setParent(this->style.getButtonStyle());
  }

  int Slider::valueToPosition(double value, int leftOffset, int rightOffset) const
  {
    int size = this->getContentWidth() - this->marker.getWidth() - leftOffset - rightOffset;
    if (value > this->getMaxValue())
      return size;
    if (value < this->getMinValue())
      return 0;
    double range = 1;
    if (this->getRange() != 0.0)
      range = this->getRange();
    return int(leftOffset + size * ((value - this->getMinValue()) / range));
  }

  double Slider::positionToValue(double position, int leftOffset, int rightOffset) const
  {
    position -= leftOffset + this->marker.getWidth() / 2;
    double size = double(this->getContentWidth() - this->marker.getWidth() - leftOffset - rightOffset);
    if (position > size)
      return this->getMaxValue();
    if (position < 0)
      return this->getMinValue();
    return (this->getRange() * (position / size)) + this->getMinValue();
  }

  void Slider::setValueStep(double length)
  {
    assert(!std::isnan(length));
    this->valueStep = length;
  }

  void Slider::setMinValue(double newValue)
  {
    if (newValue > this->max)
      throw Exception("Can't set min value to > max value.");
    this->min = newValue;
    if (newValue > this->value)
      this->value = newValue;
  }

  void Slider::setMaxValue(double newValue)
  {
    if (newValue < this->min)
      throw Exception("Can't set max value to < min value.");
    this->max = newValue;
    if (newValue < this->value)
      this->value = newValue;
  }

  void Slider::setMinMaxValues(double newMin, double newMax)
  {
    if (newMin > newMax)
      throw Exception("Can't set min > max value.");
    this->min = newMin; // to avoid exceptions
    this->setMaxValue(newMax);
    this->setMinValue(newMin);
    if (this->max != this->min && this->max - this->min <= 1 && this->valueStep == 1) // make small number sliders work by default
      this->valueStep = (this->max - this->min) / 20.0;

    assert(!std::isnan(this->valueStep));
  }

  double Slider::getFinalValue(double value) const
  {
    assert(!std::isnan(value));
    if (value < this->getMinValue())
      value = this->getMinValue();
    if (value > this->getMaxValue())
      value = this->getMaxValue();

    // in case the step is discrete to some values, round it appropriately
    if (this->discreteValues)
    {
      double diffFromNearestSmallerStep = std::fmod(value - this->min, this->valueStep);
      value -= diffFromNearestSmallerStep;
      if (diffFromNearestSmallerStep > this->valueStep / 2)
        value += this->valueStep;
    }
    return value;
  }

  void Slider::setNotchTooltips(std::vector<std::string>&& tooltips)
  {
    this->notchTooltips = std::move(tooltips);
    this->updateNotchTooltip();
  }

  void Slider::setValue(double newValue, bool dispatchChange, bool updateSlider)
  {
    newValue = this->getFinalValue(newValue);
    assert(!std::isnan(newValue));
    // I need to proceed even when newValue == this->getValue(), as the multiplier might have changed in the meantime
    // causing the widget to be positioned elsewhere
    this->setLastMarkerValue(newValue);
    this->setValueRaw(newValue);
    this->updateNotchTooltip();
    if (updateSlider)
      this->positionMarker(this->getLastMarkerValue());
    if (dispatchChange)
      this->dispatchSliderMove(newValue);
  }

  void Slider::setDiscreteSlider(bool discreteSlider)
  {
#ifdef DEBUG
    if (discreteSlider)
    {
      double mod = std::fmod(this->getRange(), this->valueStep); // can be ~0 or ~valueStep depending on float error over or under the value
      assert(mod < 0.0001 || (this->valueStep - mod) < 0.0001); // otherwise part of the slider would be inaccessible
    }
#endif
    bool enabled = discreteSlider && !this->discreteSlider;
    this->discreteSlider = discreteSlider;
    if (discreteSlider)
      this->discreteValues = true; // would not make sense otherwise
    if (enabled) // have the marker jump to position
      this->positionMarker(this->value);
  }

  void Slider::onSliderMove(GenericTargetable* owner, std::function<void(double)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnSliderMove);
    this->actionListeners.back().onSliderMove = std::move(callback);
  }

  void Slider::onSliderMove(GenericTargetable* owner, std::function<void()> callback)
  {
    return this->onSliderMove(owner, [callback = std::move(callback)](double) { callback(); });
  }

  void Slider::updateNotchTooltip()
  {
    if (this->discreteValues && this->discreteSlider)
      if (!this->notchTooltips.empty())
      {
        assert(this->notchTooltips.size() >= this->getRange() / this->getValueStep());
        uint32_t position = (this->value - this->min) / this->valueStep;
        if (this->notchTooltips.size() > position)
        {
          this->setToolTip(this->notchTooltips[position]);
          this->marker.setToolTip(this->notchTooltips[position]);
        }
      }
  }
}

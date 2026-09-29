#include "Agui/Exception.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/TextButton.hpp"
#include "Agui/Widget/DoubleSlider.hpp"
#include <Agui/ScopedSetter.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace agui
{
  DoubleSliderStyle DoubleSlider::defaultStyle;

  DoubleSlider::DoubleSlider(const DoubleSliderStyle* parentStyle)
    : Slider(parentStyle ? parentStyle : &DoubleSlider::defaultStyle, ExternalMouseDragProcessing::True)
    , highMarker(this->style.getHighButtonStyle())
    , markerPositionOffset(this->highMarker.getWidth())
  {

    this->marker.onMouseDown(this, [this](const agui::MouseEvent&) { if (this->marker.isEnabled()) this->activeMarker = ActiveMarker::Low; });
    this->marker.onMouseDrag(this, [this](const agui::MouseEvent& mouseEvent)
    {
      if (!this->shouldReactToEvent(mouseEvent))
       return;
      this->activeMarker = ActiveMarker::Low;
      const double newValue = this->positionToValue(mouseEvent.getPosition().x + mouseEvent.getSourceWidget()->getLocation().x,
                                                    0,
                                                    this->markerPositionOffset);
      const double finalValue = this->getFinalValue(newValue);
      if (finalValue != this->getValue()) // Notched sliders don't always move even when "the mouse moved"
        this->setValue(newValue, true);
    });
    this->highMarker.onMouseDrag(this, [this](const agui::MouseEvent& mouseEvent)
    {
      if (!this->shouldReactToEvent(mouseEvent))
        return;
      this->activeMarker = ActiveMarker::High;
      const double newValue = this->positionToValue(mouseEvent.getPosition().x + mouseEvent.getSourceWidget()->getLocation().x,
                                                    this->markerPositionOffset,
                                                    0);
      const double finalValue = this->getFinalValue(newValue);
      if (finalValue != this->getHighValue()) // Notched sliders don't always move even when "the mouse moved"
        this->setValue(newValue, true);
    });
    this->highMarker.onMouseDown(this, [this](const agui::MouseEvent&) { if (this->highMarker.isEnabled()) this->activeMarker = ActiveMarker::High; });
    this->highMarker.onMouseEnter(this, [this](const agui::MouseEvent& mouseEvent)
    {
      // hovering button if the mouse came from this (the DoubleSlider) and it already has a tooltip
      // this is to avoid having to wait for the mouse hover delay again when moving between a DoubleSlider and its button
      if (mouseEvent.previous == this && this->getToolTip())
        this->highMarker.mouseHover(mouseEvent);
    });

    this->highMarker.setMouseLeaveState(TextButton::ClickState::CLICKED);
    this->addPrivateChild(&this->highMarker);
    this->highMarker.setFocusable(false);
    this->highMarker.setTabable(false);
    this->setHighValue(this->getMaxValue());
  }

  void DoubleSlider::positionLowMarker(double value)
  {
    int verticalCenter = this->getContentHeight() - this->highMarker.getHeight();
    this->marker.setLocation(this->valueToPosition(value, 0, this->markerPositionOffset), verticalCenter);
  }

  void DoubleSlider::positionHighMarker(double value)
  {
    int verticalCenter = this->getContentHeight() - this->highMarker.getHeight();
    this->highMarker.setLocation(this->valueToPosition(value, this->markerPositionOffset, 0), verticalCenter);
  }

  void DoubleSlider::drawMarkers(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    super::drawMarkers(absolutePosition, enabled, graphicsContext);
    this->drawMarker(this->highMarker, absolutePosition, enabled, graphicsContext);
  }

  void DoubleSlider::drawMarkersGlowAndShadow(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext)
  {
    super::drawMarkersGlowAndShadow(absolutePosition, enabled, graphicsContext);
    this->drawMarkerGlowAndShadow(this->highMarker, absolutePosition, enabled, graphicsContext);
  }

  void DoubleSlider::drawBars(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layerType, int barTop)
  {
    int lowMarkerCenter = Slider::getMarkerCenter(this->marker);
    int highMarkerCenter = Slider::getMarkerCenter(this->highMarker);
    const ElementImageSet* emptyBar = this->isEnabled() ? this->style.getEmptyBar() : this->style.getEmptyBarDisabled();
    int emptyBarTop = int(barTop + (this->getContentHeight() - barTop - emptyBar->base.left->getHeight() * Gui::scale) / 2); // the empty bar can be thinner
    int emptyBarHeight = emptyBar->base.left->getHeight();
    // left, full part of the bar
    emptyBar->getLayer(layerType).draw(paintEvent,
                                       Rectangle(0, emptyBarTop, lowMarkerCenter, int(emptyBarHeight * Gui::scale)),
                                       absolutePosition);
    // middle, full part of the bar
    this->getFullBarElementImageSet()->getLayer(layerType).draw(paintEvent,
                                                                Rectangle(lowMarkerCenter, barTop, highMarkerCenter - lowMarkerCenter, this->getContentHeight() - barTop),
                                                                absolutePosition);
    // right, empty bar
    emptyBar->getLayer(layerType).draw(paintEvent,
                                       Rectangle(highMarkerCenter, emptyBarTop, this->getContentWidth() - highMarkerCenter, int(emptyBarHeight * Gui::scale)),
                                       absolutePosition);
  }

  bool DoubleSlider::shouldReactToEvent(const MouseEvent& mouseEvent) const
  {
    if (mouseEvent.getSourceWidget() == &this->marker && !this->marker.isEnabled())
      return false;
    if (mouseEvent.getSourceWidget() == &this->highMarker && !this->highMarker.isEnabled())
      return false;
    return true;
  }

  bool DoubleSlider::mouseDown(const MouseEvent& mouseEvent)
  {
    if (!this->shouldReactToEvent(mouseEvent))
      return false;

    // I only support disabling the low marker now
    if (!this->marker.isEnabled())
      this->activeMarker = ActiveMarker::High;

    // will move the marker on which's side we are, if between, then the last active.
    if (mouseEvent.getPosition().x <= this->marker.getLocation().x + this->marker.getWidth() && this->marker.isEnabled())
      this->activeMarker = ActiveMarker::Low;
    else if (mouseEvent.getPosition().x >= this->highMarker.getLocation().x)
      this->activeMarker = ActiveMarker::High;

    int leftOffset = 0;
    int rightOffset = 0;
    agui::Button* markerToMove = nullptr;
    if (this->activeMarker == ActiveMarker::Low)
    {
      rightOffset = this->markerPositionOffset;
      markerToMove = &this->marker;
    }
    else
    {
      leftOffset = this->markerPositionOffset;
      markerToMove = &this->highMarker;
    }

    double newValue = this->positionToValue(mouseEvent.getPosition().x - this->getContentRectangle().getLeft(),
                                            leftOffset,
                                            rightOffset);
    this->setValue(newValue, true);
    if (this->getGui())
    {
      this->getGui()->startDragging(markerToMove);
      markerToMove->mouseEnter(mouseEvent);
      markerToMove->mouseHover(mouseEvent);
    }
    return true;
  }

  bool DoubleSlider::mouseEnter(const MouseEvent& mouseEvent)
  {
    super::mouseEnter(mouseEvent);

    // hovering itself if the mouse came from the button that already has a tooltip
    // this is to avoid having to wait for the mouse hover delay again when moving between a DoubleSlider and its button
    if (mouseEvent.previous == &this->highMarker &&
        this->highMarker.getToolTip())
      this->mouseHover(mouseEvent);
    return true;
  }

  void DoubleSlider::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->positionLowMarker(this->lastMarkerValue);
    this->positionHighMarker(this->lastHighMarkerValue);
  }

  void DoubleSlider::resizeToContentsRecursive()
  {
    super::resizeToContentsRecursive();
    this->markerPositionOffset = this->highMarker.getWidth();
  }

  Widget& DoubleSlider::setEnabled(bool value)
  {
    super::setEnabled(value);
    this->highMarker.setEnabled(value);
    return *this;
  }

  void DoubleSlider::reapplySubStyles()
  {
    super::reapplySubStyles();
    this->highMarker.style.setParent(this->style.getHighButtonStyle());
  }

  void DoubleSlider::setLowValueEnabled(bool value)
  {
    this->marker.setEnabled(value);
    // If both markers are disabled, disable the whole slider widget
    // Not calling super on purpose
    Widget::setEnabled(this->highMarker.isEnabled() || value);
  }

  void DoubleSlider::setHighValueEnabled(bool value)
  {
    this->highMarker.setEnabled(value);
    // If both markers are disabled, disable the whole slider widget
    // Not calling super on purpose
    Widget::setEnabled(this->marker.isEnabled() || value);
  }

  void DoubleSlider::setMinValue(double newMin)
  {
    if (newMin > this->max)
      throw Exception("Can't set min value to > max value.");
    this->min = newMin;
    if (newMin > this->getHighValue())
      this->setHighValue(newMin);
    if (newMin > this->getLowValue())
      this->setLowValue(newMin);
  }

  void DoubleSlider::setMaxValue(double newMax)
  {
    if (newMax < this->min)
      throw Exception("Can't set max value to < min value.");
    this->max = newMax;
    if (newMax < this->getLowValue())
      this->setLowValue(newMax);
    if (newMax < this->getHighValue())
      this->setHighValue(newMax);
  }

  double DoubleSlider::getValue() const
  {
    if (this->activeMarker == ActiveMarker::Low)
      return this->getLowValue();
    return this->getHighValue();
  }

  void DoubleSlider::setValue(double value, bool dispatchChange, bool updateSlider)
  {
    if (this->activeMarker == ActiveMarker::Low)
      this->setLowValue(value, IgnoreHighValue::False, dispatchChange, updateSlider);
    else
      this->setHighValue(value, dispatchChange, updateSlider);
  }

  void DoubleSlider::setValueRaw(double value)
  {
    if (this->activeMarker == ActiveMarker::Low)
      this->value = value;
    else
      this->highValue = value;
  }

  void DoubleSlider::setLowValue(double newLowValue, IgnoreHighValue ignoreHighValue, bool dispatchChange, bool updateSlider)
  {
    if (dispatchChange && !this->marker.isEnabled())
      return;
    ScopedSetter guard(this->activeMarker);

    newLowValue = this->getFinalValue(newLowValue);

    // first check that the value we want it to set to isn't higher than the high value
    if (!ignoreHighValue && newLowValue > this->highValue)
      if (!this->highMarker.isEnabled()) // can't push a disabled marker
        newLowValue = this->highValue;
      else
      { // pushing high marker higher
        this->activeMarker = ActiveMarker::High;
        super::setValue(newLowValue, dispatchChange, updateSlider);
      }

    // now, when I know the finalValue isn't higher than the high value, I can set it
    this->activeMarker = ActiveMarker::Low;
    super::setValue(newLowValue, dispatchChange, updateSlider);
  }

  void DoubleSlider::setHighValue(double newHighValue, bool dispatchChange, bool updateSlider)
  {
    if (dispatchChange && !this->highMarker.isEnabled())
      return;
    ScopedSetter guard(this->activeMarker);

    newHighValue = this->getFinalValue(newHighValue);

    // first check that the value we want it to set to isn't lower than the low value
    if (newHighValue < this->value)
      if (!this->marker.isEnabled()) // can't push a disabled marker
        newHighValue = this->value;
      else
      { // pushing low marker lower
        this->activeMarker = ActiveMarker::Low;
        super::setValue(newHighValue, dispatchChange, updateSlider);
      }
    // now, when I know the newHighValue isn't lower than the low value, I can set it
    this->activeMarker = ActiveMarker::High;
    super::setValue(newHighValue, dispatchChange, updateSlider);
  }

  double DoubleSlider::getLastMarkerValue() const
  {
    if (this->activeMarker == ActiveMarker::Low)
      return this->lastMarkerValue;
    else
      return this->lastHighMarkerValue;
  }

  void DoubleSlider::setLastMarkerValue(double newValue)
  {
    if (this->activeMarker == ActiveMarker::Low)
      this->lastMarkerValue = newValue;
    else
      this->lastHighMarkerValue = newValue;
  }

  void DoubleSlider::setDiscreteSlider(bool discreteSlider)
  {
    bool enabled = discreteSlider && !this->discreteSlider;
    super::setDiscreteSlider(discreteSlider);
    if (enabled) // have the marker jump to position
      this->positionHighMarker(this->highValue);
  }

  bool DoubleSlider::lowerMarkerIsOnCorrectPosition() const
  {
    return this->marker.getLocation().x == this->valueToPosition(this->value, 0, this->markerPositionOffset);
  }

  void DoubleSlider::positionMarker(double value)
  {
    if (this->activeMarker == ActiveMarker::Low)
      this->positionLowMarker(value);
    else
      this->positionHighMarker(value);
  }
}

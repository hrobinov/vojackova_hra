#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/TextButton.hpp"
#include "Agui/OrientationEnum.hpp"
#include "Agui/Widget/SliderStyle.hpp"
#include <Agui/NamedBool.hpp>

namespace agui { class KeyEvent; }
namespace agui { class PaintEvent; }

namespace agui
{
  class Slider : public Widget
  {
    using super = Widget;
  public:
    using ExternalMouseDragProcessing = NamedBool<class ExternalMouseDragProcessingTag>;
    Slider(const SliderStyle* parentStyle = &Slider::defaultStyle, ExternalMouseDragProcessing externalMouseDragProcessing = ExternalMouseDragProcessing::False);
    virtual Style* getStyle() override { return &this->style; }
    virtual Slider* asSlider() override { return this; }
    virtual const Slider* asSlider() const override { return this; }

    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual void onSizeChanged(Dimension) override;

    virtual Widget& setEnabled(bool value) override;
    virtual void reapplySubStyles() override;

    int valueToPosition(double value, int leftOffset = 0, int rightOffset = 0) const; // offset - limit the marker position range
    double positionToValue(double position, int leftOffset = 0, int rightOffset = 0) const;
    double getRange() const { return this->getMaxValue() - this->getMinValue(); }
    double getMinValue() const { return this->min; }
    double getMaxValue() const { return this->max; }
    void setMinMaxValues(double min, double max);
    void setOnlyDiscreteValues(bool discreteValues) { this->discreteValues = discreteValues; }

    virtual bool handlesMouseWheel(bool) const override { return true; }
    virtual void setValueStep(double length); // Sets the length (in values) the slider moves in a key press and mouse wheel event.
    virtual double getValueStep() const { return this->valueStep; } // Length (in values) the slider moves in a key press and mouse wheel event.
    virtual void setMinValue(double value);
    virtual void setMaxValue(double value);
    virtual void setValueRaw(double value) { this->value = value; }
    virtual double getValue() const { return this->value; }
    virtual void setValue(double value, bool dispatchChange = false, bool updateSlider = true);
    virtual double getLastMarkerValue() const { return this->lastMarkerValue; }
    virtual void setLastMarkerValue(double newValue) { this->lastMarkerValue = newValue; }
    virtual void setDiscreteSlider(bool discreteSlider = true); // use discrete positions and notches
    void onSliderMove(GenericTargetable* owner, std::function<void(double)> callback);
    void onSliderMove(GenericTargetable* owner, std::function<void()> callback);
    void updateNotchTooltip();
    void setNotchTooltips(std::vector<std::string>&& tooltips);

    static int getMarkerCenter(agui::Button& marker);
    virtual Widget* getGameControllerHoveredChildInternal() override { return &this->marker; }

  protected:
    virtual void positionMarker(double value);
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void drawMarker(agui::Button& marker, const Point& absolutePosition, bool enabled, Graphics* graphicsContext);
    virtual void drawMarkerGlowAndShadow(agui::Button& marker, const Point& absolutePosition, bool enabled, Graphics* graphicsContext);
    virtual void drawMarkers(const Point& rootAbsolutePosition, bool enabled, Graphics* graphicsContext);
    virtual void drawMarkersGlowAndShadow(const Point& rootAbsolutePosition, bool enabled, Graphics* graphicsContext);
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
    virtual void recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
    virtual void handleKeyboard(const KeyEvent& keyEvent); // Left / Right or Up / Down arrow keys move the slider.
    virtual bool shouldReactToEvent(const MouseEvent& mouseEvent) const;
    void drawNotch(const PaintEvent& paintEvent, const agui::Point& absolutePosition, const ElementImageSet* notch, ElementImageSet::LayerType layer, double horizontalPos);
    void drawNotchesAndBars(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layer);
    virtual int getExtraNotchCount() const { return 0; }
    virtual void drawBars(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layerType, int barTop);
    const ElementImageSet* getFullBarElementImageSet() const;
    virtual void postDisplaySizeChanged() override;
    double getFinalValue(double value) const; // uses the limitation of min/max and discrete steps to calculate what will be the actual value of this input

  public:
    static SliderStyle defaultStyle;
    SliderStyle style;
  protected:
    std::vector<std::string> notchTooltips; // tooltips to display at positions of notched slider. Size must match the notch number.
  public:
    Button marker;

  protected:
    double value = 0; // Nothing should read this directly except virtual functions intended to operate on *this* value

    double min = 0;
    double max = 30;
    double valueStep = 1;
    double lastMarkerValue = 0; // for stability on resize
    bool discreteSlider = false; // slider can only stop at correct points
    bool discreteValues = true; // value is internally rounded to min + k * valueStep
  };
}

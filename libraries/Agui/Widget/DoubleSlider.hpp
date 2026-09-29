#pragma once
#include "Agui/Widget/Slider.hpp"

namespace agui
{
  class DoubleSlider : public Slider
  {
    using super = Slider;
  public:
    enum class ActiveMarker
    {
      Low,
      High,
    };
    DoubleSlider(const DoubleSliderStyle* parentStyle = &DoubleSlider::defaultStyle);
    virtual Style* getStyle() override { return &this->style; }

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual void resizeToContentsRecursive() override;
    virtual Widget* getGameControllerHoveredChildInternal() override { return nullptr; }

    virtual Widget& setEnabled(bool value) override;
    virtual void reapplySubStyles() override;
    virtual void setLowValueEnabled(bool value);
    virtual void setHighValueEnabled(bool value);
    virtual void setMinValue(double newMin) override;
    virtual void setMaxValue(double newMax) override;
    virtual void setDiscreteSlider(bool discreteSlider = true) override; // use discrete positions and notches
    bool lowerMarkerIsOnCorrectPosition() const;

    double getLowValue() const { return this->value; }
    double getHighValue() const { return this->highValue; }
    using IgnoreHighValue = NamedBool<class IgnoreHighValueTag>;
    void setLowValue(double value, IgnoreHighValue ignoreHighValue = IgnoreHighValue::False, bool dispatchChange = false, bool updateSlider = true);
    void setHighValue(double value, bool dispatchChange = false, bool updateSlider = true);
    ActiveMarker getActiveMarker() const { return this->activeMarker; }

    virtual double getValue() const override;
    virtual void setValue(double value, bool dispatchChange = false, bool updateSlider = true) override;
  protected:
    virtual void setValueRaw(double value) override;
    virtual double getLastMarkerValue() const override;
    virtual void setLastMarkerValue(double newValue) override;
    virtual void positionMarker(double value) override;
    virtual void positionLowMarker(double value);
    virtual void positionHighMarker(double value);
    virtual void drawMarkers(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext) override;
    virtual void drawMarkersGlowAndShadow(const agui::Point& absolutePosition, bool enabled, Graphics* graphicsContext) override;
    virtual bool shouldReactToEvent(const MouseEvent& mouseEvent) const override;
    virtual void drawBars(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layerType, int barTop) override;
    virtual int getExtraNotchCount() const override { return 1; }

  public:
    static DoubleSliderStyle defaultStyle;
    Button highMarker;

  private:
    double highValue = 0;
    double lastHighMarkerValue = 0; // for stability on resize
    ActiveMarker activeMarker = ActiveMarker::Low; // basically a global variable for some operations to decide which marker to read/move
    int markerPositionOffset = 0; // The top marker's scale is shifted by one marker width compared to the low marker; The bar is also wider by that.
  };
}

#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include <memory>

namespace agui
{
  class Slider;
  class DoubleSlider;

  class SliderStyle : public Style
  {
  private:
    using super = Style;
  public:
    explicit SliderStyle(const SliderStyle* parent = nullptr);
    explicit SliderStyle(Slider* relatedWidget, const SliderStyle* parent = nullptr);
    const SliderStyle* getParent() const { return static_cast<const SliderStyle*>(this->parent); }
    virtual void clear() override;

    const ElementImageSet* getFullBar() const { return this->getProperty(&SliderStyle::fullBar); }
    void setFullBar(const ElementImageSet& fullBar) { this->fullBar.reset(new ElementImageSet(fullBar)); }
    const ElementImageSet* getFullBarDisabled() const { return this->getProperty(&SliderStyle::fullBarDisabled); }
    void setFullBarDisabled(const ElementImageSet& fullBarDisabled) {  this->fullBarDisabled.reset(new ElementImageSet(fullBarDisabled)); }
    const ElementImageSet* getEmptyBar() const { return this->getProperty(&SliderStyle::emptyBar); }
    void setEmptyBar(const ElementImageSet& emptyBar) { this->emptyBar.reset(new ElementImageSet(emptyBar)); }
    const ElementImageSet* getEmptyBarDisabled() const { return this->getProperty(&SliderStyle::emptyBarDisabled); }
    void setEmptyBarDisabled(const ElementImageSet& emptyBarDisabled) {this->emptyBarDisabled.reset(new ElementImageSet(emptyBarDisabled));}
    bool shouldDrawNotches() const { return this->getProperty(&SliderStyle::drawNotches); }
    void setDrawNotches(bool drawNothces) { this->drawNotches = drawNothces; }
    const ElementImageSet* getNotch() const { return this->getProperty(&SliderStyle::notch); }
    void setNotch(const ElementImageSet& notch) { this->notch.reset(new ElementImageSet(notch)); }
    const ButtonStyle* getButtonStyle() const { return this->getProperty(&SliderStyle::buttonStyle); }
    ButtonStyle* initButtonStyle() { return this->initProperty(&SliderStyle::buttonStyle); }
    const ButtonStyle* getHighButtonStyle() const { return this->getProperty(&SliderStyle::highButtonStyle); }
    ButtonStyle* initHighButtonStyle() { return this->initProperty(&SliderStyle::highButtonStyle); }

  private:
    std::unique_ptr<ElementImageSet> fullBar;
    std::unique_ptr<ElementImageSet> fullBarDisabled;
    std::unique_ptr<ElementImageSet> emptyBar;
    std::unique_ptr<ElementImageSet> emptyBarDisabled;
    std::optional<bool> drawNotches; // limit value to min + k * valueStep; otherwise any double.
    std::unique_ptr<ElementImageSet> notch;
    std::unique_ptr<ButtonStyle> buttonStyle;
    std::unique_ptr<ButtonStyle> highButtonStyle; // the right button (high value) in double slider
  };

  class DoubleSliderStyle : public SliderStyle
  {
    using super = SliderStyle;
  public:
    DoubleSliderStyle(const DoubleSliderStyle* parent = nullptr);
    explicit DoubleSliderStyle(DoubleSlider* relatedWidget, const DoubleSliderStyle* parent = nullptr);
    const DoubleSliderStyle* getParent() const { return static_cast<const DoubleSliderStyle*>(this->parent); }
  };
}

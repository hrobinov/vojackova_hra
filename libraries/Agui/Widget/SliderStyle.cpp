#include "Agui/Widget/Slider.hpp"
#include "Agui/Widget/DoubleSlider.hpp"
#include "Agui/Widget/SliderStyle.hpp"
#include "Agui/Widget/TextButton.hpp"

namespace agui
{
  SliderStyle::SliderStyle(const SliderStyle* parent)
    : SliderStyle(nullptr, parent)
  {}

  SliderStyle::SliderStyle(Slider* relatedWidget, const SliderStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void SliderStyle::clear()
  {
    super::clear();
    this->fullBar.reset();
    this->fullBarDisabled.reset();
    this->emptyBar.reset();
    this->emptyBarDisabled.reset();
    this->drawNotches.reset();
    this->notch.reset();
    this->buttonStyle.reset();
    this->highButtonStyle.reset();
  }

  DoubleSliderStyle::DoubleSliderStyle(const DoubleSliderStyle* parent)
    : DoubleSliderStyle(nullptr, parent)
  {}

  DoubleSliderStyle::DoubleSliderStyle(DoubleSlider* relatedWidget, const DoubleSliderStyle* parent)
    : SliderStyle(relatedWidget, parent)
  {}
}

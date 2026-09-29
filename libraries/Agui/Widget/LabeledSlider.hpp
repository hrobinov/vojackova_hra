#pragma once
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/Slider.hpp"
#include <functional>

namespace agui
{
  // Slider with a name label and a label showing updated value
  class LabeledSlider : GenericTargetable
  {
  public:
    using ChangedCallback = std::function<void(const Widget*)>;
    using SliderValueCallback = std::function<std::string(double)>;
    LabeledSlider(std::string&& label,
                  std::string&& tooltip,
                  double min,
                  double max,
                  SliderValueCallback sliderValueCallback,
                  ChangedCallback changedCallback,
                  double change = 1.0,
                  SliderStyle* sliderStyle = nullptr);
    void updateValue(double value);
    double getValue() const;
    void addToWidget(Widget& parent);

    Label label;
    Slider slider;
    Label sliderValueLabel;

  private:
    void updateValueLabel(double value);

    ChangedCallback changedCallback;
    SliderValueCallback sliderValueCallback;
  };
}

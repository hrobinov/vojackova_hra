#include "Agui/Gui.hpp"
#include "Agui/Widget/LabeledSlider.hpp"

namespace agui
{
  LabeledSlider::LabeledSlider(std::string&& label,
                               std::string&& tooltip,
                               double min,
                               double max,
                               SliderValueCallback sliderValueCallback,
                               ChangedCallback changedCallback,
                               double change,
                               SliderStyle* sliderStyle)
    : label(std::move(label))
    , slider(sliderStyle)
    , changedCallback(changedCallback)
    , sliderValueCallback(sliderValueCallback)
  {
    this->label.setToolTip(std::move(tooltip));
    this->slider.setMinMaxValues(min, max);
    this->slider.style.setConstantWidth(150 * Gui::scale);
    if (change != 1.0)
      this->slider.setValueStep(change);
    this->slider.onSliderMove(this, [this]()
    {
      this->updateValueLabel(this->slider.getValue());
      this->changedCallback(&this->slider);
    });

    // Reserve space for the value so the window doesn't "jump" in size as values are changed.
    this->sliderValueLabel.setSizeToFit(this->sliderValueCallback(min));
    this->sliderValueLabel.setSizeToFit(this->sliderValueCallback(max));

    // Start with the label at max width
    //this->updateValue(max);
  }

  void LabeledSlider::updateValue(double value)
  {
    this->slider.setValue(value);
    this->updateValueLabel(value);
  }

  double LabeledSlider::getValue() const
  {
    return this->slider.getValue();
  }

  void LabeledSlider::addToWidget(agui::Widget& parent)
  {
    parent << this->label << this->slider << this->sliderValueLabel;
  }

  void LabeledSlider::updateValueLabel(double value)
  {
    this->sliderValueLabel.setText(this->sliderValueCallback(value));
  }
}
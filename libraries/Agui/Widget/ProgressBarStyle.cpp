#include "Agui/Widget/ProgressBarStyle.hpp"
#include "Agui/Widget/ProgressBar.hpp"
#include "Agui/Widget/VerticalFlow.hpp"

namespace agui
{
  ProgressBarStyle::ProgressBarStyle(const ProgressBarStyle* parent)
    : ProgressBarStyle(nullptr, parent)
  {}

  ProgressBarStyle::ProgressBarStyle(ProgressBar* relatedWidget, const ProgressBarStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void ProgressBarStyle::clear()
  {
    super::clear();
    this->barWidth.reset();
    this->color.reset();
    this->otherColors.reset();
    this->bar.reset();
    this->background.reset();
    this->font = nullptr;
    this->fontColor.reset();
    this->filledFontColor.reset();
    this->embedTextInBar.reset();
    this->sideTextPadding.reset();
  }

  void ProgressBarStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->barWidth)
      this->addStyleComment(result, "bar_width", std::to_string(*this->barWidth), this->propertyStatus(comparedWith, &ProgressBarStyle::barWidth));
    if (this->color)
      this->addStyleComment(result, "color", this->color->str(), this->propertyStatus(comparedWith, &ProgressBarStyle::color));
    if (this->fontColor)
      this->addStyleComment(result, "font_color", this->fontColor->str(), this->propertyStatus(comparedWith, &ProgressBarStyle::fontColor));
    if (this->sideTextPadding)
      this->addStyleComment(result, "side_text_padding", std::to_string(*this->sideTextPadding), this->propertyStatus(comparedWith, &ProgressBarStyle::sideTextPadding));
  }
}

#include "Agui/Widget/RadioButtonStyle.hpp"
#include "Agui/Widget/RadioButton.hpp"

namespace agui
{
  RadioButtonStyle::RadioButtonStyle(const RadioButtonStyle* parent)
    : RadioButtonStyle(nullptr, parent)
  {}

  RadioButtonStyle::RadioButtonStyle(RadioButton* relatedWidget, const RadioButtonStyle* parent)
   : StyleWithClickableGraphicalSet(relatedWidget, parent)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void RadioButtonStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->font)
      this->addStyleComment(result, "font", std::string(this->font->getFontName()), this->propertyStatus(comparedWith, &RadioButtonStyle::font));
    if (this->fontColor)
      this->addStyleComment(result, "font_color", this->fontColor->str(), this->propertyStatus(comparedWith, &RadioButtonStyle::fontColor));
    if (this->textPadding)
      this->addStyleComment(result, "text_padding", std::to_string(*this->textPadding), this->propertyStatus(comparedWith, &RadioButtonStyle::textPadding));
  }

  void RadioButtonStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->fontColor.reset();
    this->disabledFontColor.reset();
    this->textPadding.reset();
  }
}

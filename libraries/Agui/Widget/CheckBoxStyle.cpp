#include "Agui/Widget/CheckBox.hpp"
#include "Agui/Widget/CheckBoxStyle.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"

namespace agui
{

  CheckBoxStyle::CheckBoxStyle(const CheckBoxStyle* parent)
    : CheckBoxStyle(nullptr, parent)
  {}

  CheckBoxStyle::CheckBoxStyle(CheckBox* relatedWidget, const CheckBoxStyle* parent)
    : StyleWithClickableGraphicalSet(relatedWidget, parent)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void CheckBoxStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->font)
      this->addStyleComment(result, "font", std::string(this->font->getFontName()), this->propertyStatus(comparedWith, &CheckBoxStyle::font));
    if (this->fontColor)
      this->addStyleComment(result, "font_color", this->fontColor->str(), this->propertyStatus(comparedWith, &CheckBoxStyle::fontColor));
    if (this->disabledFontColor)
      this->addStyleComment(result, "disabled_font_color", this->disabledFontColor->str(), this->propertyStatus(comparedWith, &CheckBoxStyle::disabledFontColor));
    if (this->textPadding)
      this->addStyleComment(result, "text_padding", std::to_string(*this->textPadding), this->propertyStatus(comparedWith, &CheckBoxStyle::textPadding));
  }

  void CheckBoxStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->fontColor.reset();
    this->disabledFontColor.reset();
    this->checkmark = nullptr;
    this->disabledCheckmark = nullptr;
    this->intermediateMark = nullptr;
    this->textPadding.reset();
    this->checkboxSize.reset();
  }
}

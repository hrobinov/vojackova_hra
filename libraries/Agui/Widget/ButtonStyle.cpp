#include "Agui/Widget/TextButton.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"

namespace agui
{
  ButtonStyle::ButtonStyle(const ButtonStyle* parent)
    : ButtonStyle(nullptr, parent)
  {}

  ButtonStyle::ButtonStyle(Button* relatedWidget, const ButtonStyle* parent)
    : StyleWithClickableGraphicalSet(relatedWidget, parent)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void ButtonStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->font)
      this->addStyleComment(result, "font", std::string(this->font->getFontName()), this->propertyStatus(comparedWith, &ButtonStyle::font));
    if (this->defaultFontColor)
      this->addStyleComment(result, "default_font_color", this->defaultFontColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::defaultFontColor));
    if (this->hoveredFontColor)
      this->addStyleComment(result, "hovered_font_color", this->hoveredFontColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::hoveredFontColor));
    if (this->clickedFontColor)
      this->addStyleComment(result, "clicked_font_color", this->clickedFontColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::clickedFontColor));
    if (this->disabledFontColor)
      this->addStyleComment(result, "disabled_font_color", this->disabledFontColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::disabledFontColor));
    if (this->strikethroughColor)
      this->addStyleComment(result, "strikethrough_color", this->strikethroughColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::strikethroughColor));
    if (this->pieProgressColor)
      this->addStyleComment(result, "pie_progress_color", this->pieProgressColor->str(), this->propertyStatus(comparedWith, &ButtonStyle::pieProgressColor));
    if (this->clickedVerticalOffset)
      this->addStyleComment(result, "clicked_vertical_offset", std::to_string(*this->clickedVerticalOffset), this->propertyStatus(comparedWith, &ButtonStyle::clickedVerticalOffset));
    if (this->drawShadowUnderPicture && (*this->drawShadowUnderPicture || this->parent))
      this->addStyleComment(result, "draw_shadow_under_picture", std::to_string(*this->drawShadowUnderPicture), this->propertyStatus(comparedWith, &ButtonStyle::drawShadowUnderPicture));
    if (this->drawGrayscalePicture && (*this->drawGrayscalePicture || this->parent))
      this->addStyleComment(result, "draw_grayscale_picture", std::to_string(*this->drawGrayscalePicture), this->propertyStatus(comparedWith, &ButtonStyle::drawGrayscalePicture));
    if (this->invertColorsOfPictureWhenHoveredOrToggled && (*this->invertColorsOfPictureWhenHoveredOrToggled || this->parent))
      this->addStyleComment(result, "invert_color_of_picture_when_hovered_or_toggled", std::to_string(*this->invertColorsOfPictureWhenHoveredOrToggled), this->propertyStatus(comparedWith, &ButtonStyle::invertColorsOfPictureWhenHoveredOrToggled));
    if (this->invertColorsOfPictureWhenDisabled && (*this->invertColorsOfPictureWhenDisabled || this->parent))
      this->addStyleComment(result, "invert_color_of_picture_when_disabled", std::to_string(*this->invertColorsOfPictureWhenDisabled), this->propertyStatus(comparedWith, &ButtonStyle::invertColorsOfPictureWhenDisabled));
  }

  void ButtonStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->defaultFontColor.reset();
    this->hoveredFontColor.reset();
    this->clickedFontColor.reset();
    this->disabledFontColor.reset();
    this->selectedFontColor.reset();
    this->selectedHoveredFontColor.reset();
    this->selectedClickedFontColor.reset();
    this->strikethroughColor.reset();
    this->pieProgressColor.reset();
    this->clickedVerticalOffset.reset();
    this->drawShadowUnderPicture.reset();
    this->drawGrayscalePicture.reset();
    this->invertColorsOfPictureWhenHoveredOrToggled.reset();
    this->invertColorsOfPictureWhenDisabled.reset();
    this->iconHorizontalAlign.reset();
  }
}

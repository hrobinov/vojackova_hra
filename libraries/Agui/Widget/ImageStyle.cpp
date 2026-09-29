#include "Agui/Widget/ImageWidget.hpp"
#include "Agui/Widget/ImageStyle.hpp"
#include <Agui/StringUtil.hpp>

namespace agui
{
  ImageStyle::ImageStyle(const ImageStyle* parent)
    : ImageStyle(nullptr, parent)
  {}

  ImageStyle::ImageStyle(ImageWidget* relatedWidget, const ImageStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void ImageStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->graphicalSet && this->parent)
      this->addStyleComment(result, "graphical_set", "redefined", this->propertyStatus(comparedWith, &ImageStyle::graphicalSet));
    if (this->stretchImageToWidgetSize)
      this->addStyleComment(result, "stretch_image_to_widget_size", StringUtil::boolToString(*this->stretchImageToWidgetSize), this->propertyStatus(comparedWith, &ImageStyle::stretchImageToWidgetSize));
    if (this->invertColorsOfPictureWhenHoveredOrToggled && (*this->invertColorsOfPictureWhenHoveredOrToggled || this->parent))
      this->addStyleComment(result, "invert_color_of_picture_when_hovered_or_toggled", std::to_string(*this->invertColorsOfPictureWhenHoveredOrToggled), this->propertyStatus(comparedWith, &ImageStyle::invertColorsOfPictureWhenHoveredOrToggled));
  }

  void ImageStyle::clear()
  {
    super::clear();
    this->graphicalSet.reset();
    this->stretchImageToWidgetSize.reset();
    this->invertColorsOfPictureWhenHoveredOrToggled.reset();
  }
}

#include "Agui/Widget/Window.hpp"
#include "Agui/Widget/FrameStyle.hpp"
#include "Agui/Font.hpp"
#include <Agui/StringUtil.hpp>

namespace agui
{
  FrameStyle::FrameStyle(const FrameStyle* parent)
    : FrameStyle(nullptr, parent)
  {}

  FrameStyle::FrameStyle(Frame* relatedWidget, const FrameStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void FrameStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->useHeaderFiller)
      this->addStyleComment(result, "use_header_filler", StringUtil::boolToString(*this->useHeaderFiller), this->propertyStatus(comparedWith, &FrameStyle::useHeaderFiller));
    if (this->graphicalSet && this->parent)
      this->addStyleComment(result, "graphical_set", "redefined", this->propertyStatus(comparedWith, &FrameStyle::graphicalSet));
    if (this->headerBackground && this->parent)
      this->addStyleComment(result, "header_background", "redefined", this->propertyStatus(comparedWith, &FrameStyle::headerBackground));
    if (this->borderImageSet && this->parent)
      this->addStyleComment(result, "border_image_set", "redefined", this->propertyStatus(comparedWith, &FrameStyle::borderImageSet));
  }

  void FrameStyle::clear()
  {
    super::clear();
    this->graphicalSet.reset();
    this->useHeaderFiller.reset();
    this->dragByTitle.reset();
    this->headerBackground.reset();
    this->backgroundGraphicalSet.reset();
    this->verticalFlowStyle.reset();
    this->horizontalFlowStyle.reset();
    this->headerFlowStyle.reset();
    this->headerFillerStyle.reset();
    this->titleStyle.reset();
    this->borderImageSet.reset();
  }
}

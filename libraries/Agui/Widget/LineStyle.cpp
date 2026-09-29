#include "Agui/Widget/LineStyle.hpp"
#include "Agui/Widget/Line.hpp"

namespace agui
{
  LineStyle::LineStyle(const LineStyle* parent)
    : LineStyle(nullptr, parent)
  {}

  LineStyle::LineStyle(Line* relatedWidget, const LineStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void LineStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->borderImageSet && this->parent)
      this->addStyleComment(result, "border", "redefined", this->propertyStatus(comparedWith, &LineStyle::borderImageSet));
  }

  void LineStyle::clear()
  {
    super::clear();
    this->borderImageSet.reset();
  }

  const BorderImageSet* LineStyle::getBorder() const
  {
    const LineStyle* style = this;
    while (!style->borderImageSet && style->parent)
      style = static_cast<const LineStyle*>(style->parent);
    return style->borderImageSet.get();
  }

  void LineStyle::setBorder(const BorderImageSet & borderImageSet)
  {
    this->borderImageSet.reset(new BorderImageSet(borderImageSet));
  }
}

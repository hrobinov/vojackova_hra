#include "Agui/Widget/Flow.hpp"
#include "Agui/Widget/FlowStyle.hpp"

namespace agui
{
  FlowStyle::FlowStyle(const FlowStyle* parent)
    : FlowStyle(nullptr, parent)
  {}

  FlowStyle::FlowStyle(Layout* relatedWidget, const FlowStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void FlowStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->horizontalSpacing)
      this->addStyleComment(result, "horizontal_spacing", std::to_string(*this->horizontalSpacing), this->propertyStatus(comparedWith, &FlowStyle::horizontalSpacing));
    if (this->verticalSpacing)
      this->addStyleComment(result, "vertical_spacing", std::to_string(*this->verticalSpacing), this->propertyStatus(comparedWith, &FlowStyle::verticalSpacing));
  }

  void FlowStyle::clear()
  {
    super::clear();
    this->maxOnRow.reset();
    this->horizontalSpacing.reset();
    this->verticalSpacing.reset();
  }
}

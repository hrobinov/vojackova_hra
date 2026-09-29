#include "Agui/Widget/VerticalFlowStyle.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include "Agui/Widget/Label.hpp"
#include <cassert>

namespace agui
{
  VerticalFlowStyle::VerticalFlowStyle(const VerticalFlowStyle* parent)
    : VerticalFlowStyle(nullptr, parent)
  {
    assert(!parent || dynamic_cast<const VerticalFlowStyle*>(parent));
  }

  VerticalFlowStyle::VerticalFlowStyle(VerticalFlow* relatedWidget, const VerticalFlowStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void VerticalFlowStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->verticalSpacing)
      this->addStyleComment(result, "vertical_spacing", std::to_string(*this->verticalSpacing), this->propertyStatus(comparedWith, &VerticalFlowStyle::verticalSpacing));
  }

  void VerticalFlowStyle::clear()
  {
    super::clear();
    this->verticalSpacing.reset();
  }
}

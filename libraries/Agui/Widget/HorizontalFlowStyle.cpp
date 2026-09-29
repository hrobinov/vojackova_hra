#include "Agui/Widget/HorizontalFlow.hpp"
#include "Agui/Widget/HorizontalFlowStyle.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"

agui::HorizontalFlowStyle::HorizontalFlowStyle(const HorizontalFlowStyle* parent)
  : HorizontalFlowStyle(nullptr, parent)
{}

agui::HorizontalFlowStyle::HorizontalFlowStyle(HorizontalFlow* relatedWidget, const HorizontalFlowStyle* parent)
  : Style(relatedWidget, parent, false)
{
  if (this->relatedWidget)
    this->relatedWidget->applySizeRestrictionsInternal(*this);
}

void agui::HorizontalFlowStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
{
  super::addChangedValues(result, comparedWith);
  if (this->horizontalSpacing)
    this->addStyleComment(result, "horizontal_spacing", std::to_string(*this->horizontalSpacing), this->propertyStatus(comparedWith, &HorizontalFlowStyle::horizontalSpacing));
}

void agui::HorizontalFlowStyle::clear()
{
  super::clear();
  this->horizontalSpacing.reset();
}

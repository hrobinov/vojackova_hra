#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/EmptyWidgetStyle.hpp"

namespace agui
{
  EmptyWidgetStyle::EmptyWidgetStyle(const EmptyWidgetStyle* parent)
    : EmptyWidgetStyle(nullptr, parent)
  {}

  EmptyWidgetStyle::EmptyWidgetStyle(EmptyWidget* relatedWidget, const EmptyWidgetStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void EmptyWidgetStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->graphicalSet && this->parent)
      this->addStyleComment(result, "graphical_set", "redefined", this->propertyStatus(comparedWith, &EmptyWidgetStyle::graphicalSet));
  }
  void EmptyWidgetStyle::clear()
  {
    super::clear();
    this->graphicalSet.reset();
  }
}

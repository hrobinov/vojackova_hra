#include "Agui/StyleWithClickableGraphicalSet.hpp"

namespace agui
{
  StyleWithClickableGraphicalSet::StyleWithClickableGraphicalSet(Widget* relatedWidget, const Style* parent)
    : Style(relatedWidget, parent, false)
  {}

  void StyleWithClickableGraphicalSet::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->leftClickSound)
      this->addStyleComment(result, "left_click_sound", this->leftClickSound->getName(), this->propertyStatus(comparedWith, &StyleWithClickableGraphicalSet::leftClickSound));
    if (this->defaultGraphicalSet && this->parent)
      this->addStyleComment(result, "graphical_set", "redefined", this->propertyStatus(comparedWith, &StyleWithClickableGraphicalSet::defaultGraphicalSet));
    if (this->hoveredGraphicalSet && this->parent)
      this->addStyleComment(result, "hovered_graphical_set", "redefined", this->propertyStatus(comparedWith, &StyleWithClickableGraphicalSet::hoveredGraphicalSet));
    if (this->clickedGraphicalSet && this->parent)
      this->addStyleComment(result, "clicked_graphical_set", "redefined", this->propertyStatus(comparedWith, &StyleWithClickableGraphicalSet::clickedGraphicalSet));
    if (this->disabledGraphicalSet && this->parent)
      this->addStyleComment(result, "disabled_graphical_set", "redefined", this->propertyStatus(comparedWith, &StyleWithClickableGraphicalSet::disabledGraphicalSet));
  }

  void StyleWithClickableGraphicalSet::setLeftClickSound(std::nullptr_t)
  {
    this->leftClickSound = &EmptySound::instance;
  }

  const agui::ElementImageSet* StyleWithClickableGraphicalSet::getSelectedHoveredGraphicalSet() const
  {
    if (const ElementImageSet* result = this->getPropertyOptional(&StyleWithClickableGraphicalSet::selectedHoveredGraphicalSet))
      return result;
    return this->getSelectedGraphicalSet();
  }

  const agui::ElementImageSet* StyleWithClickableGraphicalSet::getGameControllerSelectedHoveredGraphicalSet() const
  {
    if (const ElementImageSet* result = this->getPropertyOptional(&StyleWithClickableGraphicalSet::gameControllerSelectedHoveredGraphicalSet))
      return result;
    return this->getSelectedGraphicalSet();
  }

  const agui::ElementImageSet* StyleWithClickableGraphicalSet::getSelectedClickedGraphicalSet() const
  {
    if (const ElementImageSet* result = this->getPropertyOptional(&StyleWithClickableGraphicalSet::selectedClickedGraphicalSet))
      return result;
    return this->getSelectedGraphicalSet();
  }
}

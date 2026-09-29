#include <Agui/ElementImageSet.hpp>
#include <Agui/Widget/TabStyle.hpp>
#include <Agui/Widget/Tab.hpp>
#include <stdexcept>

namespace agui
{
  TabStyle::TabStyle(const TabStyle* parent)
    : TabStyle(nullptr, parent)
  {}

  TabStyle::TabStyle(Tab* relatedWidget, const TabStyle* parent)
    : StyleWithClickableGraphicalSet(relatedWidget, parent)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void TabStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->badgeFont = nullptr;
    this->badgeHorizontalSpacing.reset();
    this->defaultFontColor.reset();
    this->selectedFontColor.reset();
    this->disabledFontColor.reset();
    this->defaultBadgeFontColor.reset();
    this->selectedBadgeFontColor.reset();
    this->disabledBadgeFontColor.reset();
    this->gameControllerSelectedHoveredGraphicalSet.reset();
    this->overrideGraphicsOnEdges.reset();
    this->increaseHeightWhenSelected.reset();
    this->leftClickSound = nullptr;
    this->leftEdgeSelectedGraphicalSet.reset();
    this->rightEdgeSelectedGraphicalSet.reset();
    this->defaultBadgeGraphicalSet.reset();
    this->selectedBadgeGraphicalSet.reset();
    this->hoverBadgeGraphicalSet.reset();
    this->pressBadgeGraphicalSet.reset();
    this->disabledBadgeGraphicalSet.reset();
    this->drawGrayscalePicture.reset();
  }

  const ElementImageSet* TabStyle::getLocalGraphicalSet(GraphicalSetType type) const
  {
    switch (type)
    {
      case GraphicalSetType::Default: return this->defaultGraphicalSet.get();
      case GraphicalSetType::Selected: return this->selectedGraphicalSet.get();
      case GraphicalSetType::Hover: return this->hoveredGraphicalSet.get();
      case GraphicalSetType::GameControllerSelectedHover: return this->gameControllerSelectedHoveredGraphicalSet.get();
      case GraphicalSetType::Press: return this->clickedGraphicalSet.get();
      case GraphicalSetType::Disabled: return this->disabledGraphicalSet.get();
      case GraphicalSetType::LeftEdgeSelected: return this->leftEdgeSelectedGraphicalSet.get();
      case GraphicalSetType::RightEdgeSelected: return this->rightEdgeSelectedGraphicalSet.get();
    }
    return nullptr;
  }

  const ElementImageSet* TabStyle::getLocalBadgeGraphicalSet(GraphicalSetType type) const
  {
    switch (type)
    {
      case GraphicalSetType::Default: return this->defaultBadgeGraphicalSet.get();
      case GraphicalSetType::Selected: return this->selectedBadgeGraphicalSet.get();
      case GraphicalSetType::Hover: return this->hoverBadgeGraphicalSet.get();
      case GraphicalSetType::Press: return this->pressBadgeGraphicalSet.get();
      case GraphicalSetType::Disabled: return this->disabledBadgeGraphicalSet.get();
      case GraphicalSetType::GameControllerSelectedHover:
      case GraphicalSetType::LeftEdgeSelected:
      case GraphicalSetType::RightEdgeSelected:
        return nullptr;
    }
    return nullptr;
  }

  const ElementImageSet* TabStyle::getGraphicalSet(GraphicalSetType type) const
  {
    const TabStyle* style = this;
    const ElementImageSet* imageSet = nullptr;
    while (!(imageSet = style->getLocalGraphicalSet(type)))
      style = static_cast<const TabStyle*>(style->parent);
    return imageSet;
  }

  const ElementImageSet* TabStyle::getBadgeGraphicalSet(GraphicalSetType type) const
  {
    const TabStyle* style = this;
    const ElementImageSet* imageSet = nullptr;
    while (!(imageSet = style->getLocalBadgeGraphicalSet(type)))
      style = static_cast<const TabStyle*>(style->parent);
    return imageSet;
  }

  void TabStyle::initGraphicalSet(GraphicalSetType type, const ElementImageSet& graphicalSet)
  {
    switch (type)
    {
      case GraphicalSetType::Default:
      case GraphicalSetType::Selected:
      case GraphicalSetType::Hover:
      case GraphicalSetType::GameControllerSelectedHover:
      case GraphicalSetType::Press:
      case GraphicalSetType::Disabled: throw std::runtime_error("should be initialised by parent instead");
      case GraphicalSetType::LeftEdgeSelected: this->leftEdgeSelectedGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::RightEdgeSelected: this->rightEdgeSelectedGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
    }
  }

  void TabStyle::initBadgeGraphicalSet(GraphicalSetType type, const ElementImageSet& graphicalSet)
  {
    switch (type)
    {
      case GraphicalSetType::Default: this->defaultBadgeGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::Selected: this->selectedBadgeGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::Hover: this->hoverBadgeGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::Press: this->pressBadgeGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::Disabled: this->disabledBadgeGraphicalSet.reset(new ElementImageSet(graphicalSet)); break;
      case GraphicalSetType::GameControllerSelectedHover:
      case GraphicalSetType::LeftEdgeSelected:
      case GraphicalSetType::RightEdgeSelected:
        return;
    }
  }
}

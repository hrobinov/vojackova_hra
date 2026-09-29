#pragma once
#include "Agui/ElementImageSet.hpp"
#include "Agui/Sound.hpp"
#include "Agui/Style.hpp"
#include <memory>

namespace agui
{
  class StyleWithClickableGraphicalSet : public Style
  {
    using super = Style;
  public:
    StyleWithClickableGraphicalSet(Widget* relatedWidget, const Style* parent);
    const StyleWithClickableGraphicalSet* getParent() const { return static_cast<const StyleWithClickableGraphicalSet*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;

    const Sound* getLeftClickSound() const { return this->getPropertyOptional(&StyleWithClickableGraphicalSet::leftClickSound); }
    void setLeftClickSound(const Sound* leftClickSound) { this->leftClickSound = leftClickSound; }
    void setLeftClickSound(std::nullptr_t);
    const ElementImageSet* getDefaultGraphicalSet() const { return this->getProperty(&StyleWithClickableGraphicalSet::defaultGraphicalSet); }
    void setDefaultGraphicalSet(const ElementImageSet* defaultGraphicalSet) { this->defaultGraphicalSet.reset(new ElementImageSet(*defaultGraphicalSet)); }
    const ElementImageSet* getHoveredGraphicalSet() const { return this->getProperty(&StyleWithClickableGraphicalSet::hoveredGraphicalSet); }
    void setHoveredGraphicalSet(const ElementImageSet* hoveredGraphicalSet) { this->hoveredGraphicalSet.reset(new ElementImageSet(*hoveredGraphicalSet)); }
    const ElementImageSet* getClickedGraphicalSet() const { return this->getProperty(&StyleWithClickableGraphicalSet::clickedGraphicalSet); }
    void setClickedGraphicalSet(const ElementImageSet* clickedGraphicalSet) { this->clickedGraphicalSet.reset(new ElementImageSet(*clickedGraphicalSet)); }
    const ElementImageSet* getDisabledGraphicalSet() const { return this->getProperty(&StyleWithClickableGraphicalSet::disabledGraphicalSet); }
    void setDisabledGraphicalSet(const ElementImageSet* disabledGraphicalSet) { this->disabledGraphicalSet.reset(new ElementImageSet(*disabledGraphicalSet)); }
    const ElementImageSet* getSelectedGraphicalSet() const { return this->getProperty(&StyleWithClickableGraphicalSet::selectedGraphicalSet); }
    void setSelectedGraphicalSet(const ElementImageSet* selectedGraphicalSet) { this->selectedGraphicalSet.reset(new ElementImageSet(*selectedGraphicalSet)); }
    const ElementImageSet* getSelectedHoveredGraphicalSet() const;
    void setSelectedHoveredGraphicalSet(const ElementImageSet* selectedHoveredGraphicalSet) { this->selectedHoveredGraphicalSet.reset(new ElementImageSet(*selectedHoveredGraphicalSet)); }
    const ElementImageSet* getGameControllerSelectedHoveredGraphicalSet() const;
    void setGameControllerSelectedHoveredGraphicalSet(const ElementImageSet* gameControllerSelectedHoveredGraphicalSet) { this->gameControllerSelectedHoveredGraphicalSet.reset(new ElementImageSet(*gameControllerSelectedHoveredGraphicalSet)); }
    const ElementImageSet* getSelectedClickedGraphicalSet() const;
    void setSelectedClickedGraphicalSet(const ElementImageSet* selectedClickedGraphicalSet) { this->selectedClickedGraphicalSet.reset(new ElementImageSet(*selectedClickedGraphicalSet)); }

  protected:
    const Sound* leftClickSound = nullptr;
    std::unique_ptr<ElementImageSet> defaultGraphicalSet;
    std::unique_ptr<ElementImageSet> hoveredGraphicalSet;
    std::unique_ptr<ElementImageSet> clickedGraphicalSet;
    std::unique_ptr<ElementImageSet> disabledGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedHoveredGraphicalSet;
    std::unique_ptr<ElementImageSet> gameControllerSelectedHoveredGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedClickedGraphicalSet;
  };
}

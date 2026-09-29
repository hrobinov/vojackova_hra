#pragma once
#include <Agui/Color.hpp>
#include <Agui/StyleWithClickableGraphicalSet.hpp>
#include <memory>
#include <Agui/Sound.hpp>

namespace agui
{
  class Font;
  class Tab;

  class TabStyle : public StyleWithClickableGraphicalSet
  {
    using super = StyleWithClickableGraphicalSet;
  public:
    enum class GraphicalSetType
    {
      Default,
      Selected,
      Hover,
      GameControllerSelectedHover,
      Press,
      Disabled,
      LeftEdgeSelected,
      RightEdgeSelected
    };
    explicit TabStyle(const TabStyle* parent = nullptr);
    explicit TabStyle(Tab* relatedWidget, const TabStyle* parent = nullptr);
    const TabStyle* getParent() const { return static_cast<const TabStyle*>(this->parent); }
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&TabStyle::font); }
    const Font* getBadgeFont() const { return this->getProperty(&TabStyle::badgeFont); }
    void setFont(const Font* font) { this->font = font; }
    void setBadgeFont(const Font* font) { this->badgeFont = font; }

    int16_t getBadgeHorizontalSpacing() const { return this->getProperty(&TabStyle::badgeHorizontalSpacing); }
    void setBadgeHorizontalSpacing(int16_t value) { this->setProperty(&TabStyle::badgeHorizontalSpacing, value); }

    const Color& getDefaultFontColor() const { return *this->getProperty(&TabStyle::defaultFontColor); }
    void setDefaultFontColor(const Color& fontColor) { this->defaultFontColor.reset(new Color(fontColor)); }
    const Color& getSelectedFontColor() const  { return *this->getProperty(&TabStyle::selectedFontColor); }
    void setSelectedFontColor(const Color& fontColor) { this->selectedFontColor.reset(new Color(fontColor)); }
    const Color& getDisabledFontColor() const  { return *this->getProperty(&TabStyle::disabledFontColor); }
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }

    const Color& getDefaultBadgeFontColor() const { return *this->getProperty(&TabStyle::defaultBadgeFontColor); }
    void setDefaultBadgeFontColor(const Color& fontColor) { this->defaultBadgeFontColor.reset(new Color(fontColor)); }
    const Color& getSelectedBadgeFontColor() const  { return *this->getProperty(&TabStyle::selectedBadgeFontColor); }
    void setSelectedBadgeFontColor(const Color& fontColor) { this->selectedBadgeFontColor.reset(new Color(fontColor)); }
    const Color& getDisabledBadgeFontColor() const  { return *this->getProperty(&TabStyle::disabledBadgeFontColor); }
    void setDisabledBadgeFontColor(const Color& disabledFontColor) { this->disabledBadgeFontColor.reset(new Color(disabledFontColor)); }
    bool getDrawGrayscalePicture() const { return this->getProperty(&TabStyle::drawGrayscalePicture); }
    void setDrawGrayscalePicture(bool drawGrayscalePicture) { this->drawGrayscalePicture = drawGrayscalePicture; }

    const ElementImageSet* getGraphicalSet(GraphicalSetType type) const;
    const ElementImageSet* getBadgeGraphicalSet(GraphicalSetType type) const;
    void initGraphicalSet(GraphicalSetType type, const ElementImageSet& graphicalSet);
    void initBadgeGraphicalSet(GraphicalSetType type, const ElementImageSet& graphicalSet);

    bool shouldOverrideGraphicsOnEdges() const { return this->getProperty(&TabStyle::overrideGraphicsOnEdges); }
    void setOverrideGraphicsOnEdges(bool overrideGraphicsOnEdges) { this->overrideGraphicsOnEdges = overrideGraphicsOnEdges; }
    bool shouldIncreaseHeightWhenSelected() const { return this->getProperty(&TabStyle::increaseHeightWhenSelected); }
    void setIncreaseHeightWhenSelected(bool increaseHeightWhenSelected) { this->increaseHeightWhenSelected = increaseHeightWhenSelected; }

    const Sound* getLeftClickSound() const { return this->getPropertyOptional(&TabStyle::leftClickSound); }
    void setLeftClickSound(const Sound* leftClickSound) { this->leftClickSound = leftClickSound; }

  private:
    const ElementImageSet* getLocalGraphicalSet(GraphicalSetType type) const; // only from this class
    const ElementImageSet* getLocalBadgeGraphicalSet(GraphicalSetType type) const; // only from this class

    const Font* font = nullptr;
    const Font* badgeFont = nullptr;

    std::optional<int16_t> badgeHorizontalSpacing;

    std::unique_ptr<Color> defaultFontColor;
    std::unique_ptr<Color> selectedFontColor;
    std::unique_ptr<Color> disabledFontColor;

    std::unique_ptr<Color> defaultBadgeFontColor;
    std::unique_ptr<Color> selectedBadgeFontColor;
    std::unique_ptr<Color> disabledBadgeFontColor;

    // replaces selectedGraphicalSet when the tab is the first/last. Should be used when tab container has 0 padding to connect to the frame.
    std::optional<bool> overrideGraphicsOnEdges;
    // To create the overlap of tabs to 'absorb into' the frame under it.
    std::optional<bool> increaseHeightWhenSelected;

    std::unique_ptr<ElementImageSet> leftEdgeSelectedGraphicalSet;
    std::unique_ptr<ElementImageSet> rightEdgeSelectedGraphicalSet;

    std::unique_ptr<ElementImageSet> defaultBadgeGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedBadgeGraphicalSet;
    std::unique_ptr<ElementImageSet> hoverBadgeGraphicalSet;
    std::unique_ptr<ElementImageSet> pressBadgeGraphicalSet;
    std::unique_ptr<ElementImageSet> disabledBadgeGraphicalSet;
    std::optional<bool> drawGrayscalePicture;
  };
}

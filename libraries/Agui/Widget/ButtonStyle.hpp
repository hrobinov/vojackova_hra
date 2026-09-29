#pragma once
#include "Agui/Color.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/StyleWithClickableGraphicalSet.hpp"

namespace agui
{
  class Font;
  class Button;

  class ButtonStyle : public StyleWithClickableGraphicalSet
  {
    using super = StyleWithClickableGraphicalSet;
  public:
    explicit ButtonStyle(const ButtonStyle* parent = nullptr);
    explicit ButtonStyle(Button* relatedWidget, const ButtonStyle* parent = nullptr);
    const ButtonStyle* getParent() const { return static_cast<const ButtonStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&ButtonStyle::font); }
    void setFont(const Font* font) { this->setProperty(&ButtonStyle::font, font); }
    const Color& getDefaultFontColor() const { return *this->getProperty(&ButtonStyle::defaultFontColor); }
    void setDefaultFontColor(const Color& fontColor) { this->defaultFontColor.reset(new Color(fontColor)); }
    const Color& getHoveredFontColor() const { return *this->getProperty(&ButtonStyle::hoveredFontColor); }
    void setHoveredFontColor(const Color& hoveredFontColor) { this->hoveredFontColor.reset(new Color(hoveredFontColor)); }
    const Color& getClickedFontColor() const { return *this->getProperty(&ButtonStyle::clickedFontColor); }
    void setClickedFontColor(const Color& clickedFontColor) { this->clickedFontColor.reset(new Color(clickedFontColor)); }
    const Color& getDisabledFontColor() const { return *this->getProperty(&ButtonStyle::disabledFontColor); }
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }
    const Color& getSelectedFontColor() const { return *this->getProperty(&ButtonStyle::selectedFontColor); }
    void setSelectedFontColor(const Color& selectedFontColor) { this->selectedFontColor.reset(new Color(selectedFontColor)); }
    const Color& getSelectedHoveredFontColor() const { return *this->getProperty(&ButtonStyle::selectedHoveredFontColor); }
    void setSelectedHoveredFontColor(const Color& selectedHoveredFontColor) { this->selectedHoveredFontColor.reset(new Color(selectedHoveredFontColor)); }
    const Color& getSelectedClickedFontColor() const { return *this->getProperty(&ButtonStyle::selectedClickedFontColor); }
    void setSelectedClickedFontColor(const Color& selectedClickedFontColor) { this->selectedClickedFontColor.reset(new Color(selectedClickedFontColor)); }
    const Color& getStrikethroughColor() const { return *this->getProperty(&ButtonStyle::strikethroughColor); }
    void setStrikethroughColor(const Color& strikethroughColor) { this->strikethroughColor.reset(new Color(strikethroughColor)); }
    const Color& getPieProgressColor() const { return *this->getProperty(&ButtonStyle::pieProgressColor); }
    void setPieProgressColor(const Color& pieProgressColor) { this->pieProgressColor.reset(new Color(pieProgressColor)); }
    int getClickedVerticalOffset() const { return this->getProperty(&ButtonStyle::clickedVerticalOffset); }
    void setClickedVerticalOffset(int clickedLabelVerticalOffset) { this->clickedVerticalOffset = clickedLabelVerticalOffset; }
    bool getDrawShadowUnderPicture() const { return this->getProperty(&ButtonStyle::drawShadowUnderPicture); }
    void setDrawShadowUnderPicture(bool drawShadowUnderPicture) { this->drawShadowUnderPicture = drawShadowUnderPicture; }
    bool getDrawGrayscalePicture() const { return this->getProperty(&ButtonStyle::drawGrayscalePicture); }
    void setDrawGrayscalePicture(bool drawGrayscalePicture) { this->drawGrayscalePicture = drawGrayscalePicture; }
    bool getInvertColorsOfPictureWhenHoveredOrToggled() const { return this->getProperty(&ButtonStyle::invertColorsOfPictureWhenHoveredOrToggled); }
    void setInvertColorsOfPictureWhenHoveredOrToggled(bool invertColorsOfPictureWhenHoveredOrToggled) { this->invertColorsOfPictureWhenHoveredOrToggled = invertColorsOfPictureWhenHoveredOrToggled; }
    bool getInvertColorsOfPictureWhenDisabled() const { return this->getProperty(&ButtonStyle::invertColorsOfPictureWhenDisabled); }
    void setInvertColorsOfPictureWhenDisabled(bool invertColorsOfPictureWhenDisabled) { this->invertColorsOfPictureWhenDisabled = invertColorsOfPictureWhenDisabled; }
    HorizontalAlign getIconHorizontalAlign() const { return this->getProperty(&ButtonStyle::iconHorizontalAlign); }
    void setIconHorizontalAlign(HorizontalAlign iconHorizontalAlign) { this->iconHorizontalAlign = iconHorizontalAlign; }

  private:
    const Font* font = nullptr;
    std::unique_ptr<Color> defaultFontColor;
    std::unique_ptr<Color> hoveredFontColor;
    std::unique_ptr<Color> clickedFontColor;
    std::unique_ptr<Color> disabledFontColor;
    std::unique_ptr<Color> selectedFontColor;
    std::unique_ptr<Color> selectedHoveredFontColor;
    std::unique_ptr<Color> selectedClickedFontColor;
    std::unique_ptr<Color> strikethroughColor;
    std::unique_ptr<Color> pieProgressColor;
    std::optional<int> clickedVerticalOffset;
    std::optional<bool> drawShadowUnderPicture;
    std::optional<bool> drawGrayscalePicture;
    std::optional<bool> invertColorsOfPictureWhenHoveredOrToggled;
    std::optional<bool> invertColorsOfPictureWhenDisabled;
    std::optional<HorizontalAlign> iconHorizontalAlign;
  };
}

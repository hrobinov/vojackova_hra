#pragma once
#include "Agui/Color.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/StyleWithClickableGraphicalSet.hpp"
#include <memory>
#include <optional>

namespace agui
{
  class Font;
  class RadioButton;

  class RadioButtonStyle : public StyleWithClickableGraphicalSet
  {
    using super = StyleWithClickableGraphicalSet;
  public:
    enum class BackgroundType
    {
      Default,
      Selected,
      Hover,
      Press,
      Disabled
    };
    explicit RadioButtonStyle(const RadioButtonStyle* parent = nullptr);
    explicit RadioButtonStyle(RadioButton* relatedWidget, const RadioButtonStyle* parent = nullptr);
    const RadioButtonStyle* getParent() const { return static_cast<const RadioButtonStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&RadioButtonStyle::font); }
    void setFont(const Font* font) { this->setProperty(&RadioButtonStyle::font, font); }
    const Color& getFontColor() const { return *this->getProperty(&RadioButtonStyle::fontColor); }
    void setFontColor(const Color& fontColor) { this->fontColor.reset(new Color(fontColor)); }
    const Color& getDisabledFontColor() const { return *this->getProperty(&RadioButtonStyle::disabledFontColor); }
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }
    uint32_t getTextPadding() const { return this->getProperty(&RadioButtonStyle::textPadding); }
    void setTextPadding(uint32_t textPadding) { this->textPadding = textPadding; }

  private:
    const Font* font = nullptr;
    std::unique_ptr<Color> fontColor;
    std::unique_ptr<Color> disabledFontColor;
    std::optional<uint32_t> textPadding;
  };
}

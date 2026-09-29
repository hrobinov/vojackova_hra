#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/TextEnums.hpp"
#include <memory>

namespace agui
{
  class Font;
  class Label;

  class LabelStyle : public Style
  {
    using super = Style;
  public:
    explicit LabelStyle(const LabelStyle* parent = nullptr);
    explicit LabelStyle(Label* relatedWidget, const LabelStyle* parent = nullptr);
    explicit LabelStyle(Label* relatedWidget, const LabelStyle* parent, RichTextSetting richTextSetting);
    const LabelStyle* getParent() const { return static_cast<const LabelStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&LabelStyle::font); }
    void setFont(const Font* font) { this->setProperty(&LabelStyle::font, font); }
    const Color& getFontColor() const { return *this->getProperty(&LabelStyle::fontColor); }
    void setFontColor(const Color& fontColor) { this->fontColor.reset(new Color(fontColor)); }
    const Color& getHoveredFontColor() const;
    void setHoveredFontColor(const Color& hoveredFontColor) { this->hoveredFontColor.reset(new Color(hoveredFontColor)); }
    const Color& getGameControllerHoveredFontColor() const;
    void setGameControllerHoveredFontColor(const Color& gameControllerHoveredFontColor) { this->gameControllerHoveredFontColor.reset(new Color(gameControllerHoveredFontColor)); }
    const Color& getClickedFontColor() const;
    void setClickedFontColor(const Color& clickedFontColor) { this->clickedFontColor.reset(new Color(clickedFontColor)); }
    const Color& getDisabledFontColor() const;
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }
    const Color& getParentHoveredColor() const;
    void setParentHoveredColor(const Color& parentHoveredColor) { this->parentHoveredColor.reset(new Color(parentHoveredColor)); }
    bool isSingleLine() const;
    void setSingleLine(bool value);
    bool isStrikethrough() const;
    void setStrikethrough(bool value);
    bool isUnderlined() const;
    void setUnderlined(bool value);
    virtual void setParent(const Style* parent) override;
    const RichTextSetting& getRichTextSetting() const { return *this->getProperty(&LabelStyle::richTextSetting); }
    void setRichTextSetting(const RichTextSetting& richTextSetting);
    const Color& getRichTextHighlightErrorColor() const { return *this->getProperty(&LabelStyle::richTextHighlightErrorColor); }
    void setRichTextHighlightErrorColor(const Color& color) { this->richTextHighlightErrorColor.reset(new Color(color)); }
    const Color& getRichTextHighlightWarningColor() const { return *this->getProperty(&LabelStyle::richTextHighlightWarningColor); }
    void setRichTextHighlightWarningColor(const Color& color) { this->richTextHighlightWarningColor.reset(new Color(color)); }
    const Color& getRichTextHighlightOkColor() const { return *this->getProperty(&LabelStyle::richTextHighlightOkColor); }
    void setRichTextHighlightOkColor(const Color& color) { this->richTextHighlightOkColor.reset(new Color(color)); }
    bool labelFlagValueIsTheSame(const LabelStyle* other, uint8_t labelFlag) const;
    PropertyStatus labelPropertyStatus(const LabelStyle* comparedWith, uint8_t flag) const;

    static std::optional<RichTextSetting> overrideRichTextSetting;

  private:
    void checkApplyOverrideRichTextSetting();

    constexpr static uint8_t SINGLE_LINE_DEFINED = 1 << 0;
    constexpr static uint8_t SINGLE_LINE_VALUE = 1 << 1;
    constexpr static uint8_t STRIKETHROUGH = 1 << 2;
    constexpr static uint8_t UNDERLINED = 1 << 3;

    const Font* font = nullptr;
    std::unique_ptr<Color> fontColor;
    std::unique_ptr<Color> hoveredFontColor;
    std::unique_ptr<Color> gameControllerHoveredFontColor;
    std::unique_ptr<Color> clickedFontColor;
    std::unique_ptr<Color> disabledFontColor;
    std::unique_ptr<Color> parentHoveredColor;
    std::unique_ptr<RichTextSetting> richTextSetting;
    std::unique_ptr<Color> richTextHighlightErrorColor;
    std::unique_ptr<Color> richTextHighlightWarningColor;
    std::unique_ptr<Color> richTextHighlightOkColor;
    uint8_t labelFlags = 0;
  };
}

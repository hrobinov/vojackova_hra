#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Font.hpp"
#include "Agui/TextEnums.hpp"

#include <memory>

namespace agui
{
  class Font;
  class TextBox;

  class TextBoxStyle : public Style
  {
  private:
    using super = Style;
  public:
    explicit TextBoxStyle(const TextBoxStyle* parent = nullptr);
    explicit TextBoxStyle(TextBox* relatedWidget, const TextBoxStyle* parent);
    virtual ~TextBoxStyle() = default;
    const TextBoxStyle* getParent() const { return static_cast<const TextBoxStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&TextBoxStyle::font); }
    void setFont(const Font* font);
    const Color& getFontColor() const { return *this->getProperty(&TextBoxStyle::fontColor); }
    void setFontColor(const Color& fontColor) { this->fontColor.reset(new Color(fontColor)); }
    const Color& getDisabledFontColor() const { return *this->getProperty(&TextBoxStyle::disabledFontColor); }
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }
    const Color& getSelectionBackgroundColor() const { return *this->getProperty(&TextBoxStyle::selectionBackgroundColor); }
    void setSelectionBackgroundColor(const Color& selectionBackgroundColor) { this->selectionBackgroundColor.reset(new Color(selectionBackgroundColor)); }
    virtual const ElementImageSet* getDefaultBackground() const { return this->getProperty(&TextBoxStyle::defaultBackground); }
    virtual void setDefaultBackground(const ElementImageSet& defaultBackground) { this->defaultBackground.reset(new ElementImageSet(defaultBackground)); }
    virtual const ElementImageSet* getActiveBackground() const { return this->getProperty(&TextBoxStyle::activeBackground); }
    virtual void setActiveBackground(const ElementImageSet& activeBackground) { this->activeBackground.reset(new ElementImageSet(activeBackground)); }
    virtual const ElementImageSet* getGameControllerHoveredBackground() const { return this->getProperty(&TextBoxStyle::gameControllerHoveredBackground); }
    virtual void setGameControllerHoveredBackground(const ElementImageSet& gameControllerHoveredBackground) { this->gameControllerHoveredBackground.reset(new ElementImageSet(gameControllerHoveredBackground)); }
    virtual const ElementImageSet* getDisabledBackground() const { return this->getProperty(&TextBoxStyle::disabledBackground); }
    virtual void setDisabledBackground(const ElementImageSet& disabledBackground) { this->disabledBackground.reset(new ElementImageSet(disabledBackground)); }
    const RichTextSetting& getRichTextSettings() const { return *this->getProperty(&TextBoxStyle::richTextSetting); }
    void setRichTextSettings(const RichTextSetting& settings);
    const Color& getRichTextHighlightErrorColor() const { return *this->getProperty(&TextBoxStyle::richTextHighlightErrorColor); }
    void setRichTextHighlightErrorColor(const Color& color) { this->richTextHighlightErrorColor.reset(new Color(color)); }
    const Color& getRichTextHighlightWarningColor() const { return *this->getProperty(&TextBoxStyle::richTextHighlightWarningColor); }
    void setRichTextHighlightWarningColor(const Color& color) { this->richTextHighlightWarningColor.reset(new Color(color)); }
    const Color& getRichTextHighlightOkColor() const { return *this->getProperty(&TextBoxStyle::richTextHighlightOkColor); }
    void setRichTextHighlightOkColor(const Color& color) { this->richTextHighlightOkColor.reset(new Color(color)); }
    const Color& getSelectedRichTextHighlightErrorColor() const { return *this->getProperty(&TextBoxStyle::selectedRichTextHighlightErrorColor); }
    void setSelectedRichTextHighlightErrorColor(const Color& color) { this->selectedRichTextHighlightErrorColor.reset(new Color(color)); }
    const Color& getSelectedRichTextHighlightWarningColor() const { return *this->getProperty(&TextBoxStyle::selectedRichTextHighlightWarningColor); }
    void setSelectedRichTextHighlightWarningColor(const Color& color) { this->selectedRichTextHighlightWarningColor.reset(new Color(color)); }
    const Color& getSelectedRichTextHighlightOkColor() const { return *this->getProperty(&TextBoxStyle::selectedRichTextHighlightOkColor); }
    void setSelectedRichTextHighlightOkColor(const Color& color) { this->selectedRichTextHighlightOkColor.reset(new Color(color)); }
    virtual void setParent(const Style* parent) override;

  private:
    const Font* font = nullptr;
    std::unique_ptr<Color> fontColor;
    std::unique_ptr<Color> disabledFontColor;
    std::unique_ptr<Color> selectionBackgroundColor;
    std::unique_ptr<ElementImageSet> defaultBackground;
    std::unique_ptr<ElementImageSet> activeBackground;
    std::unique_ptr<ElementImageSet> gameControllerHoveredBackground;
    std::unique_ptr<ElementImageSet> disabledBackground;
    std::unique_ptr<RichTextSetting> richTextSetting;
    std::unique_ptr<Color> richTextHighlightErrorColor;
    std::unique_ptr<Color> richTextHighlightWarningColor;
    std::unique_ptr<Color> richTextHighlightOkColor;
    std::unique_ptr<Color> selectedRichTextHighlightErrorColor;
    std::unique_ptr<Color> selectedRichTextHighlightWarningColor;
    std::unique_ptr<Color> selectedRichTextHighlightOkColor;
  };
}

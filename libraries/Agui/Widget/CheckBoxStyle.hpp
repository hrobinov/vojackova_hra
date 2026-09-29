#pragma once
#include "Agui/Color.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/StyleWithClickableGraphicalSet.hpp"
#include <memory>
#include <optional>

namespace agui
{
  class Font;
  class CheckBox;

  class CheckBoxStyle : public StyleWithClickableGraphicalSet
  {
    using super = StyleWithClickableGraphicalSet;
  public:
    explicit CheckBoxStyle(const CheckBoxStyle* parent = nullptr);
    explicit CheckBoxStyle(CheckBox* relatedWidget, const CheckBoxStyle* parent = nullptr);
    const CheckBoxStyle* getParent() const { return static_cast<const CheckBoxStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const Font* getFont() const { return this->getProperty(&CheckBoxStyle::font); }
    void setFont(const Font* font) { this->setProperty(&CheckBoxStyle::font, font); }
    const Color& getFontColor() const { return *this->getProperty(&CheckBoxStyle::fontColor); }
    void setFontColor(const Color& fontColor) { this->fontColor.reset(new Color(fontColor)); }
    const Color& getDisabledFontColor() const { return *this->getProperty(&CheckBoxStyle::disabledFontColor); }
    void setDisabledFontColor(const Color& disabledFontColor) { this->disabledFontColor.reset(new Color(disabledFontColor)); }
    const Image* getCheckmark() const { return this->getProperty(&CheckBoxStyle::checkmark); }
    void setCheckmark(Image* checkmark) { this->checkmark = checkmark; }
    const Image* getDisabledCheckmark() const { return this->getProperty(&CheckBoxStyle::disabledCheckmark); }
    void setDisabledCheckmark(Image* disabledCheckmark) { this->disabledCheckmark = disabledCheckmark; }
    const Image* getIntermediateMark() const { return this->getProperty(&CheckBoxStyle::intermediateMark); }
    void setIntermediateMark(Image* intermediateMark) { this->intermediateMark = intermediateMark; }
    uint32_t getTextPadding() const { return this->getProperty(&CheckBoxStyle::textPadding); }
    void setTextPadding(uint32_t textPadding) { this->setProperty(&CheckBoxStyle::textPadding, textPadding); }

  private:
    const Font* font = nullptr;
    std::unique_ptr<Color> fontColor;
    std::unique_ptr<Color> disabledFontColor;
    Image* checkmark = nullptr;
    Image* disabledCheckmark = nullptr;
    Image* intermediateMark = nullptr;
    std::optional<uint32_t> textPadding;
    std::unique_ptr<Dimension> checkboxSize;
  };
}

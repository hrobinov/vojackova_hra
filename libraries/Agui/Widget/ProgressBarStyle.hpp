#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include <vector>

namespace agui
{
  class Font;
  class ProgressBar;
  class ToolTip;

  class ProgressBarStyle : public Style
  {
  private:
    using super = Style;
  public:
    class OtherColor
    {
    public:
      OtherColor(double valueLessThan, std::optional<Color> color, ElementImageSet* bar)
        : valueLessThan(valueLessThan)
        , color(color)
      {
        if (bar)
          this->bar.emplace(*bar);
      }
      double valueLessThan;
      std::optional<Color> color;
      std::optional<ElementImageSet> bar;
    };
    using OtherColors = std::vector<OtherColor>;

    explicit ProgressBarStyle(const ProgressBarStyle* parent = nullptr);
    explicit ProgressBarStyle(ProgressBar* relatedWidget, const ProgressBarStyle* parent = nullptr);
    const ProgressBarStyle* getParent() const { return static_cast<const ProgressBarStyle*>(this->parent); }
    virtual void clear() override;

    uint32_t getBarWidth() const { return this->getProperty(&ProgressBarStyle::barWidth); }
    void setBarWidth(uint32_t barWidth) { this->setProperty(&ProgressBarStyle::barWidth, barWidth); }
    const Color& getColor() const { return *this->getProperty(&ProgressBarStyle::color); }
    void setColor(const Color& color) { this->color.reset(new Color(color)); }
    const OtherColors& getOtherColors() const { return *this->getProperty(&ProgressBarStyle::otherColors); }
    void setOtherColors(OtherColors&& otherColors) { this->otherColors.reset(new OtherColors(std::move(otherColors))); }
    const ElementImageSet* getBar() const { return this->getProperty(&ProgressBarStyle::bar); }
    void setBar(ElementImageSet* bar) { this->bar = std::make_unique<ElementImageSet>(*bar); }
    const ElementImageSet* getBackground() const { return this->getProperty(&ProgressBarStyle::background); }
    void setBackground(ElementImageSet* background) { this->background = std::make_unique<ElementImageSet>(*background); }
    const Font* getFont() const { return this->getProperty(&ProgressBarStyle::font); }
    void setFont(const Font* font) { this->setProperty(&ProgressBarStyle::font, font); }
    const Color& getFontColor() const { return *this->getProperty(&ProgressBarStyle::fontColor); }
    void setFontColor(const Color& fontColor) { this->fontColor.reset(new Color(fontColor)); }
    const Color* getFilledFontColor() const { return this->getPropertyOptional(&ProgressBarStyle::filledFontColor); }
    void setFilledFontColor(const Color& color) { this->filledFontColor.reset(new Color(color)); }
    bool getEmbedTextInBar() const { return this->getProperty(&ProgressBarStyle::embedTextInBar); }
    void setEmbedTextInBar(bool embedTextInBar) { this->setProperty(&ProgressBarStyle::embedTextInBar, embedTextInBar); }
    int16_t getSideTextPadding() const { return this->getProperty(&ProgressBarStyle::sideTextPadding); }
    void setSideTextPadding(int16_t sideTextPadding) { this->setProperty(&ProgressBarStyle::sideTextPadding, sideTextPadding); }

    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;

  private:
    std::optional<uint32_t> barWidth;
    std::unique_ptr<Color> color;
    std::unique_ptr<OtherColors> otherColors;
    std::unique_ptr<ElementImageSet> bar;
    std::unique_ptr<ElementImageSet> background;
    const Font* font = nullptr;
    std::unique_ptr<Color> fontColor;
    std::unique_ptr<Color> filledFontColor;
    std::optional<bool> embedTextInBar;
    std::optional<int16_t> sideTextPadding;
  };
}

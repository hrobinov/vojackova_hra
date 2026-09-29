#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include <vector>

namespace agui
{
  class Font;
  class ActivityBar;

  class ActivityBarStyle : public Style
  {
  private:
    using super = Style;
  public:
    explicit ActivityBarStyle(const ActivityBarStyle* parent = nullptr);
    explicit ActivityBarStyle(ActivityBar* relatedWidget, const ActivityBarStyle* parent = nullptr);
    const ActivityBarStyle* getParent() const { return static_cast<const ActivityBarStyle*>(this->parent); }
    virtual void clear() override;

    float getSpeed() const { return this->getProperty(&ActivityBarStyle::speed); }
    void setSpeed(float speed) { this->speed = speed; }
    uint32_t getBarWidth() const { return this->getProperty(&ActivityBarStyle::barWidth); }
    void setBarWidth(uint32_t barWidth) { this->setProperty(&ActivityBarStyle::barWidth, barWidth); }
    const Color& getColor() { return *this->getProperty(&ActivityBarStyle::color); }
    void setColor(const Color& color) { this->color.reset(new Color(color)); }
    const ElementImageSet* getBar() { return this->getProperty(&ActivityBarStyle::bar); }
    void setBar(const ElementImageSet* bar) { this->bar = bar; }
    float getBarSizeRatio() const { return this->getProperty(&ActivityBarStyle::barSizeRatio); }
    void setBarSizeRatio(float barSizeRatio) { this->barSizeRatio = barSizeRatio; }
    const ElementImageSet* getBackground() { return this->getProperty(&ActivityBarStyle::background); }
    void setBackground(const ElementImageSet *background) { this->background = background; }

  private:
    std::optional<float> speed; // percentage of total size
    std::optional<uint32_t> barWidth;
    std::unique_ptr<Color> color;
    const ElementImageSet* bar = nullptr;
    std::optional<float> barSizeRatio; // percentage of total size
    const ElementImageSet* background = nullptr;
  };
}

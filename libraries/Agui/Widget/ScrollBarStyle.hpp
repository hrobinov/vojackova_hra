#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include <memory>

namespace agui
{
  class ScrollBarStyle : public Style
  {
  private:
    using super = Style;
  public:
    explicit ScrollBarStyle(const ScrollBarStyle* parent = nullptr);
    explicit ScrollBarStyle(Widget* relatedWidget, const ScrollBarStyle* parent = nullptr);
    const ScrollBarStyle* getParent() const { return static_cast<const ScrollBarStyle*>(this->parent); }
    virtual void clear() override;

    const ButtonStyle* getThumbButtonStyle() const { return this->getProperty(&ScrollBarStyle::thumbButtonStyle); }
    ButtonStyle* initThumbButtonStyle() { return this->initProperty(&ScrollBarStyle::thumbButtonStyle); }
    const ElementImageSet* getBackgroundGraphicalSet() const { return this->getProperty(&ScrollBarStyle::backgroundGraphicalSet); }
    void setBackgroundGraphicalSet(const ElementImageSet* backgroundGraphicalSet) { this->backgroundGraphicalSet.reset(new ElementImageSet(*backgroundGraphicalSet)); }

  private:
    std::unique_ptr<ButtonStyle> thumbButtonStyle;
    std::unique_ptr<ElementImageSet> backgroundGraphicalSet;
  };
}

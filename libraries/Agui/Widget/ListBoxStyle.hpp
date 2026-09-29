#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/Sound.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include "Agui/Widget/ScrollPaneStyle.hpp"
#include <memory>

namespace agui
{
  class Font;
  class ListBox;

  class ListBoxStyle : public Style
  {
    using super = Style;
  public:
    explicit ListBoxStyle(const ListBoxStyle* parent = nullptr);
    explicit ListBoxStyle(ListBox* relatedWidget, const ListBoxStyle* parent = nullptr);
    const ListBoxStyle* getParent() const { return static_cast<const ListBoxStyle*>(this->parent); }
    virtual void clear() override;

    const ButtonStyle* getItemStyle() const { return this->getProperty(&ListBoxStyle::itemStyle); }
    ButtonStyle* initItemStyle() { return this->initProperty(&ListBoxStyle::itemStyle); }
    const ScrollPaneStyle* getScrollPaneStyle() const { return this->getProperty(&ListBoxStyle::scrollPaneStyle); }
    ScrollPaneStyle* initScrollPaneStyle() { return this->initProperty(&ListBoxStyle::scrollPaneStyle); }

  private:
    std::unique_ptr<ButtonStyle> itemStyle;
    std::unique_ptr<ScrollPaneStyle> scrollPaneStyle;
  };
}

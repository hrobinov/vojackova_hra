#pragma once
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Pusher.hpp"
#include "Agui/Layout.hpp"
#include "Agui/Widget/HorizontalFlowStyle.hpp"
#include "Agui/AlignmentEnum.hpp"
#include <memory>

namespace agui
{
  class HorizontalFlow : public Layout
  {
  public:
    explicit HorizontalFlow(const HorizontalFlowStyle* parentStyle = &HorizontalFlow::defaultStyle);
    explicit HorizontalFlow(VerticalAlign align);
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override;
    virtual Style* getStyle() override { return &this->style; }
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;
    HorizontalFlow& stretchHorizontally() { Widget::stretchHorizontally(); return *this; } // to keep the return type
    HorizontalFlow& stretchVertically() { Widget::stretchVertically(); return *this; } // to keep the return type
    HorizontalFlow& operator<<(const Pusher& pusher);
    HorizontalFlow& operator<<(Widget& other) { this->add(&other); return *this; }
    HorizontalFlow& operator<<(Widget* other) { this->add(other); return *this; }
    template<class T> requires std::is_base_of_v<Widget, T>
    HorizontalFlow& operator<<(std::unique_ptr<T>& other) { this->add(other.get()); return *this; }
    HorizontalFlow& centerVertically();
    virtual HorizontalFlow& dontShrinkInReactionToSetSize() override { Widget::dontShrinkInReactionToSetSize(); return *this; }

    static HorizontalFlowStyle defaultStyle;
    HorizontalFlowStyle style;
  };
}

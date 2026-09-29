#pragma once
#include "Agui/Pusher.hpp"
#include "Agui/Layout.hpp"
#include "Agui/Widget/VerticalFlowStyle.hpp"
#include "Agui/AlignmentEnum.hpp"

namespace agui
{
  class VerticalFlow : public Layout
  {
  public:
    explicit VerticalFlow(const VerticalFlowStyle* parentStyle = &VerticalFlow::defaultStyle);
    explicit VerticalFlow(HorizontalAlign align);
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override;
    virtual Style* getStyle() override { return &this->style; }
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;
    VerticalFlow& stretchHorizontally() { Widget::stretchHorizontally(); return *this; } // to keep the return type
    VerticalFlow& stretchVertically() { Widget::stretchVertically(); return *this; } // to keep the return type
    VerticalFlow& operator<<(const Pusher& pusher);
    VerticalFlow& operator<<(Widget& other) { this->add(&other); return *this; }
    VerticalFlow& operator<<(Widget* other) { this->add(other); return *this; }
    template<class T> requires std::is_base_of_v<Widget, T>
    VerticalFlow& operator<<(std::unique_ptr<T>& other) { this->add(other.get()); return *this; }
    virtual VerticalFlow& dontShrinkInReactionToSetSize() override { Widget::dontShrinkInReactionToSetSize(); return *this; }
    VerticalFlow& centerVertically();

    static VerticalFlowStyle defaultStyle;
    VerticalFlowStyle style;
    bool squashToWidestShrinkingWidget = true;
  };
}

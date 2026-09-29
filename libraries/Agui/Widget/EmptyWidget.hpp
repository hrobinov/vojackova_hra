#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/EmptyWidgetStyle.hpp"
#include "Agui/Style.hpp"
namespace agui
{
  /** Simple dummy class for a widget */
  class EmptyWidget : public Widget
  {
    using super = Widget;
  public:
    EmptyWidget(const EmptyWidgetStyle* parentStyle = &EmptyWidget::defaultStyle);
    virtual ~EmptyWidget();
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual Style* getStyle() override { return &this->style; }
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;

    virtual agui::Widget& setDragTarget(Window* dragTarget);
    virtual bool isMovable() const;
    virtual Window* getDragTarget() override;
    virtual const Window* getDragTarget() const override;

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;

    static EmptyWidgetStyle defaultStyle;

    EmptyWidgetStyle style;
    Window* dragTarget = nullptr;
    double fixedRatio = 0;
  };
}

#pragma once
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/ActivityBarStyle.hpp"

namespace agui
{
  class Widget;
  class PaintEvent;
  class ActivityBar : public agui::EmptyWidget
  {
  public:
    static ActivityBarStyle defaultStyle;

    ActivityBar(const ActivityBarStyle* parentStyle = & ActivityBar::defaultStyle);
    virtual ~ActivityBar() = default;
    virtual void paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundShadow(const agui::PaintEvent &paintEvent, const agui::Point &absolutePosition) override;
    virtual void logic(double timeElapsed) override;
    virtual void resizeToContents() override;
    virtual Style *getStyle() override { return &this->style; }
    Rectangle getBarRectangle() const;

    float leftPositionRatio = 0;
    float rightPositionRatio = 0;
    ActivityBarStyle style;
    double lastUpdate = 0;
  };
}

#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/GraphStyle.hpp"

namespace agui
{
  class BezierPlot final : public Widget
  {
    using super = Widget;
  public:
    BezierPlot(const GraphStyle* parentStyle = nullptr);
    virtual Style* getStyle() override { return &this->style; }
  protected:
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point&) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point&) override;
    virtual void onSizeChanged(Dimension) override;
    void update();
  public:
    static GraphStyle defaultStyle;
    GraphStyle style;
    bool clampValues = false;
    Rectangle graphRectangle;
    Rectangle dataRectangle = Rectangle(-30, -10, 60, 20);
    uint32_t colorIndex = 0;
    std::vector<Point> dataPoints;
    std::vector<Color> colorPoints;
  };
}

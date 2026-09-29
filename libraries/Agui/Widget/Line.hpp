#pragma once
#include "Agui/GuiDirection.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/LineStyle.hpp"

namespace agui
{
  class Line : public Widget
  {
  public:
    Line() = delete;
  protected:
    Line(GuiDirection direction, const LineStyle* parentStyle = &Line::defaultStyle);
    virtual void resizeToContents() override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point&) override;
  public:
    virtual Style* getStyle() override { return &this->style; }

    LineStyle style;
    static LineStyle defaultStyle;

  private:
    GuiDirection direction;
  };

  class HorizontalLine : public Line
  {
  public:
    HorizontalLine(const LineStyle* parentStyle = &Line::defaultStyle)
      : Line(GuiDirection::Horizontal, parentStyle) {}
  };

  class VerticalLine : public Line
  {
  public:
    VerticalLine(const LineStyle* parentStyle = &Line::defaultStyle)
      : Line(GuiDirection::Vertical, parentStyle) {}
  };
}

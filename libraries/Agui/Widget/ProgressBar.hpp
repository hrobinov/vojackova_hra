#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/ProgressBarStyle.hpp"
#include "Agui/GuiDirection.hpp"
namespace agui
{
  class Widget;
  class PaintEvent;

  enum class HasText { No, Yes };

  class ProgressBar : public agui::Widget
  {
  public:
    ProgressBar(const ProgressBarStyle* parentStyle = &ProgressBar::defaultStyle,
                GuiDirection direction = GuiDirection::Horizontal,
                HasText hasText = HasText::No);
    virtual ~ProgressBar() = default;
    virtual void paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundShadow(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual Style* getStyle() override { return &this->style; }
    virtual void resizeToContents() override;
    void setValue(double value);
    double getValue() const { return this->value; }
    static ProgressBarStyle defaultStyle;

    ProgressBarStyle style;
  private:
    Rectangle getBarRectangle() const;
    bool reserveSpaceForText() const;

    double value = 0;
    GuiDirection direction;
    bool hasText;  // reserve space for text even if the text is empty
  };
}

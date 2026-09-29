#pragma once
#include <Agui/Widget/Button.hpp>
namespace agui { class Layout; }

namespace agui
{
  class LayoutButton : public Button
  {
    using super = Button;
  public:
    LayoutButton(GuiDirection guiDirection, const ButtonStyle* parentStyle = &Button::defaultStyle);
    virtual bool empty() const override;
    virtual void add(Widget* widget) override;
    virtual void insert(Widget* widget, uint32_t index) override;
    virtual void addFront(Widget* widget) override;
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False) override;
    virtual void clear() override;
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    virtual void changeClickState(Clickable::ClickState state) override;

    // Nothing in the layout will be selectable, as all the actions need to be related to this button
    virtual Widget* getWidgetUnderMouse(Point, Point, Rectangle, TransparentValue) override { return this; }
    virtual void resizeToContents() override;

    VerticalFlow* getVerticalFlow();
    HorizontalFlow* getHorizontalFlow();

    GuiDirection direction;
    Layout* layout;
  };
}

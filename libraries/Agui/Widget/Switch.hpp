#pragma once
#include "Agui/Clickable.hpp"
#include "Agui/Widget/SwitchStyle.hpp"
#include "Agui/GuiDirection.hpp"
#include "Agui/Widget/SwitchState.hpp"
namespace agui
{
  class Widget;
  class PaintEvent;
  class Switch : public Clickable
  {
    using super = Clickable;
  public:
    Switch(SwitchStyle* parentStyle = &Switch::defaultStyle);
    virtual ~Switch() = default;
    virtual void paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual Style* getStyle() override { return &this->style; }
    virtual void resizeToContents() override;
    SwitchState getState() const { return this->state; }
    void setState(SwitchState state);
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    void onSwitchToggle(GenericTargetable* owner, std::function<void()> callback);
    void dispatchSwitchToggle();

    static SwitchStyle defaultStyle;
    SwitchStyle style;

    bool allowNoneState = false;
  private:
    SwitchState state = SwitchState::Left;
  };
}

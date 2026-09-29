#pragma once
#include "Agui/ResizableText.hpp"
#include "Agui/Widget.hpp"

namespace agui
{
  /** Manages mouse based states - hover and clicked. */
  class Clickable : public Widget
  {
    using super = Widget;
  public:
    enum class ClickState
    {
      DEFAULT,
      HOVERED,
      CLICKED
    };

  protected:
    virtual void changeClickState(Clickable::ClickState state); // Internally changes the widget's state.
  public:
    Clickable() = default;
    /** Determines the state of the button when the mouse leaves and the mouse is down. */
    void setMouseLeaveState(Clickable::ClickState state);
    /** @return The state of the button when the mouse leaves and the mouse is down. */
    Clickable::ClickState getMouseLeaveState() const;
    Clickable::ClickState getClickState() const;
    Clickable::ClickState getClickStateForRendering() const;
    bool isMouseInside() const;
    bool isMouseDown() const { return this->mouseIsDown; }
    void setClickState(Clickable::ClickState state); // Manually sets the button state. Will be changed when the button gets an event.
    void setRenderAsHovered(bool value) { this->renderAsHovered = value; }
    void resetMouseState();
    virtual void modifyClickState(); // Changes the button's state based on the mouse.
    virtual void onClickStateChanged() {}
    virtual void focusGained(TabbedIn tabbedIn) override;
    virtual void focusLost() override;
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool shouldPlaySound() const { return true; }
    virtual Widget* getWidgetUnderMouse(Point, Point, Rectangle, TransparentValue) override { return this; } // internal widgets are ignored

    virtual bool isToggleButton() const { return false; }
    virtual bool isToggled() const { return false; }
    virtual Widget& setEnabled(bool enabled) override;
    virtual Widget* getGameControllerHoveredChildInternal() override { return this; }

    virtual Clickable* asClickable() override { return this; }

    static constexpr Clickable::ClickState defaultState = ClickState::DEFAULT;

  protected:
    Clickable::ClickState state = defaultState;
    bool mouseIsInside = false;
    bool renderAsHovered = false;
    bool mouseIsDown = false;
    bool clickStateLocked = false; // State can't be changed by modifyClickState as long as this is on.
    bool inputDisabled = false;
    Clickable::ClickState mouseLeaveState = defaultState;
  };
}

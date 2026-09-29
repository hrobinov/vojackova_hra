#pragma once
#include "Agui/Point.hpp"
#include "Agui/MouseButton.hpp"
namespace agui { class Widget; }

namespace agui
{
  class MouseEvent
  {
  public:
    enum class Type
    {
      NONE,
      MOUSE_DOWN,
      MOUSE_UP,
      MOUSE_MOVE,
      MOUSE_CLICK,
      MOUSE_DOUBLE_CLICK,
      MOUSE_WHEEL_UP,
      MOUSE_WHEEL_DOWN,
      MOUSE_WHEEL_LEFT,
      MOUSE_WHEEL_RIGHT,
      MOUSE_ENTER,
      MOUSE_LEAVE,
      MOUSE_HOVER,
      MOUSE_DRAG,
      MOUSE_MODAL_DOWN,
      MOUSE_MODAL_UP
    };

    MouseEvent() = default;
    MouseEvent(Widget* source);
    MouseEvent(Widget* source, MouseButton mouseButton, Type eventType = Type::MOUSE_CLICK);
    MouseEvent(Widget* source, Point  position, MouseButton mouseButton, Type eventType);
    MouseEvent(Point position) : position(position) {}
    /** The position must already be relative to the source. */
    MouseEvent(const Point& position,
               int mouseWheelChange,
               MouseButton button,
               Type eventType,
               double timeStamp = 0,
               bool isAlt = false,
               bool isControl = false,
               bool isShift = false,
               Widget* source = nullptr,
               Widget* previous = nullptr);

    Point getPosition() const; // The position of the mouse when the event occurred relative to the widget it came from.
    Point getAbsolutePosition() const;
    int getMouseWheelChange() const; // The vertical mouse wheel change (Delta Z). It can be negative.
    MouseButton getButton() const; // The mouse button that was pressed down, or released.
    Type getEvent() const;
    double getTimeStamp() const; // How much time the application had been running when the event occurred.
    bool alt() const; // If alt was pressed when the event occurred.
    bool control() const; // If control was pressed when the event occurred.
    bool shift() const; // If shift was pressed when the event occurred. */
    Widget* getSourceWidget() const;
    MouseEvent copyWithNewSource(Widget* newSource) const;
    MouseEvent copyWithNewType(Type newType) const;
  private:
    Point position;
    int mouseWheelChange = 0;
    MouseButton button = MouseButton::NONE;
    Type eventType = Type::NONE;
    double timeStamp = 0;

    bool isAlt = false;
    bool isControl = false;
    bool isShift = false;
    Widget* source = nullptr;
  public:
    Widget* previous = nullptr;
  };
}

#include "Agui/MouseEvent.hpp"
#include "Agui/Widget.hpp"

namespace agui
{
  MouseEvent::MouseEvent(const Point& position,
                         int mouseWheelChange,
                         MouseButton button,
                         Type eventType,
                         double timeStamp,
                         bool isAlt, bool isControl,
                         bool isShift,
                         Widget* source,
                         Widget* previous)
    : position(position)
    , mouseWheelChange(mouseWheelChange)
    , button(button)
    , eventType(eventType)
    , timeStamp(timeStamp)
    , isAlt(isAlt)
    , isControl(isControl)
    , isShift(isShift)
    , source(source)
    , previous(previous)
  {}

  MouseEvent::MouseEvent(Widget* source)
    : source(source)
  {}

  MouseEvent::MouseEvent(Widget* source, MouseButton mouseButton, Type eventType)
    : button(mouseButton)
    , eventType(eventType)
    , source(source)
  {}

  MouseEvent::MouseEvent(Widget* source, Point position, MouseButton mouseButton, Type eventType)
    : position(position)
    , button(mouseButton)
    , eventType(eventType)
    , source(source)
  {}

  Point MouseEvent::getPosition() const
  {
    return this->position;
  }

  Point MouseEvent::getAbsolutePosition() const
  {
    return this->position + this->source->getAbsolutePosition();
  }

  int MouseEvent::getMouseWheelChange() const
  {
    return this->mouseWheelChange;
  }

  MouseButton MouseEvent::getButton() const
  {
    return this->button;
  }

  MouseEvent::Type MouseEvent::getEvent() const
  {
    return this->eventType;
  }

  double MouseEvent::getTimeStamp() const
  {
    return this->timeStamp;
  }

  bool MouseEvent::alt() const
  {
    return this->isAlt;
  }

  bool MouseEvent::control() const
  {
    return this->isControl;
  }

  bool MouseEvent::shift() const
  {
    return this->isShift;
  }

  Widget* MouseEvent::getSourceWidget() const
  {
    return this->source;
  }

  MouseEvent MouseEvent::copyWithNewSource(Widget* newSource) const
  {
    MouseEvent result(*this);
    if (newSource)
      result.position = this->position + this->source->getAbsolutePosition() - newSource->getAbsolutePosition();
    result.source = newSource;
    return result;
  }

  MouseEvent MouseEvent::copyWithNewType(Type newType) const
  {
    MouseEvent result(*this);
    result.eventType = newType;
    return result;
  }
}

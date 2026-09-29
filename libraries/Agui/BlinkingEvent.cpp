#include "Agui/BlinkingEvent.hpp"

namespace agui
{
  void BlinkingEvent::processBlinkEvent(double elapsedTime)
  {
    if (elapsedTime > this->lastBlinkTime)
    {
      this->blinking = !this->blinking;
      this->lastBlinkTime = elapsedTime + this->blinkInterval;
    }
    else if (this->blinkNeedsInvalidation)
    {
      this->lastBlinkTime = elapsedTime + this->blinkInterval;
      this->blinkNeedsInvalidation = false;
    }
  }

  bool BlinkingEvent::isBlinking() const
  {
    return this->blinking;
  }

  void BlinkingEvent::invalidateBlink()
  {
    this->blinkNeedsInvalidation = true;
  }

  void BlinkingEvent::setBlinking(bool blinking)
  {
    this->blinking = blinking;
  }

  void BlinkingEvent::setBlinkingInverval(double interval)
  {
    this->blinkInterval = interval;
    this->blinkNeedsInvalidation = true;
  }
}

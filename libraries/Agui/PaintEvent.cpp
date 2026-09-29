#include "Agui/PaintEvent.hpp"

namespace agui
{
  PaintEvent::PaintEvent(bool enabled, Graphics* g)
  {
    this->enabled = enabled;
    this->graphicsContext = g;
  }

  bool PaintEvent::isEnabled() const
  {
    return this->enabled;
  }

  Graphics* PaintEvent::graphics() const
  {
    return this->graphicsContext;
  }
}

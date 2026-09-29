#pragma once
namespace agui
{
  class Graphics;
}

namespace agui
{
  class PaintEvent final
  {
  public:
    PaintEvent() = default;
    ~PaintEvent() = default;
    PaintEvent(bool enabled, Graphics* g);
    /** Although the widget itself may be enabled,
     * if any of its parents are disabled, all the children are inherently disabled.
     * @return Whether the widget should be drawn with an enabled or disabled look. */
    bool isEnabled() const;
    /** @return The graphics context used to call drawing methods. */
    Graphics* graphics() const;

  private:
    bool enabled = true;
    Graphics* graphicsContext = nullptr;
  };
}

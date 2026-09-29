#pragma once
#include "Agui/Widget.hpp"

namespace agui
{
  class Layout : public Widget
  {
  public:
    Layout() = default;
    /** This is what should be called to update the layout. You should never call layoutChildren directly. */
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;

    virtual Window* getDragTarget() override;
    virtual const Window* getDragTarget() const override;

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    /** Sets whether or not dragging the caption bar (top of the frame) will result in moving the Frame. */
    virtual void setDragTarget(Window* dragTarget);
    /** @return True if dragging the caption bar (top of the frame) will result in moving the Frame. */
    virtual bool isMovable() const;
    virtual bool dragOnlyByLeftMouseButton() const override { return true; }
    virtual void resizeToContents() override;
    virtual TransparentValue isTransparent() const override { return TransparentValue::DependsOnChildren; }
  protected:
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;
    virtual void layoutChildren(SetSizeInfo setSizeInfo) = 0;
    virtual void paintBackground(const PaintEvent& paintEvent, const Point& absolutePosition) override;

  public:
    double fixedRatio = 0;
  private:
    Window* dragTarget = nullptr;
  };
}

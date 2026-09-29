#include "Agui/Layout.hpp"

#include <Agui/DrawCulledChildren.hpp>

#include "Agui/Widget/Window.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"

namespace agui
{
  Window* Layout::getDragTarget()
  {
    if (this->dragTarget)
      return this->dragTarget->getDragTarget();
    return nullptr;
  }

  const Window* Layout::getDragTarget() const
  {
    if (this->dragTarget)
      return this->dragTarget->getDragTarget();
    return nullptr;
  }

  void Layout::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
   if (this->fixedRatio != 0)
   {
      // making sure that the aspect ratio is kept the same when it is being squashed
      if (setSizeInfo.vertical == Change::Squashing)
        width = height * this->fixedRatio;
      if (setSizeInfo.horizontal == Change::Squashing)
      {
        int newHeight = width / fixedRatio;
        if (newHeight < height)
          height = newHeight;
        else
          width = height * fixedRatio;
      }
    }
   setSizeInfo.reactionToSetSize = ReactionToSetSize::True;
    Widget::setSize(width, height, setSizeInfo);
    this->layoutChildren(setSizeInfo);
  }

  void Layout::paintBackground(const PaintEvent& paintEvent, const agui::Point&)
  {
    if (paintEvent.graphics()->debugView)
      paintEvent.graphics()->drawRectangle(this->getSizeRectangle(), agui::Color(1, 0, 0));
  }

  void Layout::setDragTarget(Window* dragTarget)
  {
    this->dragTarget = dragTarget;
  }

  bool Layout::isMovable() const
  {
    return this->getDragTarget() != nullptr;
  }

  void Layout::resizeToContents()
  {
    this->layoutChildren(SetSizeInfo(ReactionToSetSize::False));
  }

  void Layout::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    //if (graphicsContext->pushClippingRect(this, this->getSizeRectangle()))
    if (!graphicsContext->isClippingRectEmpty())
    {
      this->paint(PaintEvent(enabled, graphicsContext), absolutePosition);

      const int leftPadding = this->getLeftPadding();
      const int topPadding = this->getTopPadding();

      for (Widget* widget : this->getPrivateChildren())
        if (widget->isVisible() && widget->shouldRender())
          widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));

      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });
    }
    //graphicsContext->popClippingRect();
  }

  bool Layout::mouseDown(const MouseEvent& mouseEvent)
  {
    if (this->dragTarget)
      return this->dragTarget->mouseDown(mouseEvent);
    return false;
  }

  bool Layout::mouseUp(const MouseEvent& mouseEvent)
  {
    if (this->dragTarget)
      return this->dragTarget->mouseUp(mouseEvent);
    return false;
  }

  bool Layout::mouseDrag(const MouseEvent& mouseEvent)
  {
    Widget::mouseDrag(mouseEvent);
    if (this->dragTarget)
      return this->dragTarget->mouseDrag(mouseEvent);
    return false;
  }
}

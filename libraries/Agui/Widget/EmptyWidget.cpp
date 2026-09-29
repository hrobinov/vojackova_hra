#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/Window.hpp"

namespace agui
{
  EmptyWidgetStyle EmptyWidget::defaultStyle;

  EmptyWidget::EmptyWidget(const EmptyWidgetStyle* parentStyle)
    : style(this, parentStyle)
  {}

  EmptyWidget::~EmptyWidget()
  {}

  void EmptyWidget::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->shadowView)
      this->style.getGraphicalSet()->shadow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void EmptyWidget::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getGraphicalSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void EmptyWidget::paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->glowView)
      this->style.getGraphicalSet()->glow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  agui::Window* EmptyWidget::getDragTarget()
  {
    return this->dragTarget;
  }

  const agui::Window* EmptyWidget::getDragTarget() const
  {
    return this->dragTarget;
  }

  bool EmptyWidget::mouseDown(const MouseEvent& mouseEvent)
  {
    if (this->dragTarget)
      return this->dragTarget->mouseDown(mouseEvent);
    return false;
  }

  bool EmptyWidget::mouseUp(const MouseEvent& mouseEvent)
  {
    if (this->dragTarget)
      return this->dragTarget->mouseUp(mouseEvent);
    return false;
  }

  bool EmptyWidget::mouseDrag(const MouseEvent& mouseEvent)
  {
    Widget::mouseDrag(mouseEvent);
    if (this->dragTarget)
      return this->dragTarget->mouseDrag(mouseEvent);
    return false;
  }

  void EmptyWidget::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    if (this->fixedRatio != 0)
    {
      if (setSizeInfo.vertical == Change::Squashing)
        width = height * fixedRatio;
      if (setSizeInfo.horizontal == Change::Squashing)
      {
        int newHeight = width / fixedRatio;
        if (newHeight < height)
          height = newHeight;
        else
          width = height * fixedRatio;
      }
      if (setSizeInfo.horizontal == Change::Stretching)
        height = width / fixedRatio;
    }
    super::setSize(width, height, setSizeInfo);
  }

  agui::Widget& EmptyWidget::setDragTarget(Window* dragTarget)
  {
    this->dragTarget = dragTarget;
    return *this;
  }

  bool EmptyWidget::isMovable() const
  {
    return this->getDragTarget() != nullptr;
  }
}

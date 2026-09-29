#pragma once
#include <Agui/Graphics.hpp>
#include <Agui/Widget.hpp>

namespace agui
{
  template<class DrawCallback>
  void drawCulledChildren(Widget& parent,
                          Graphics* graphicsContext,
                          const Point& absolutePosition,
                          const int leftPadding,
                          const int topPadding,
                          DrawCallback drawCallback)
  {
    if (!parent.isCullingDrawing())
    {
      for (Widget* widget : agui::Widget::VisibleChildren(&parent))
        if (widget->shouldRender())
          drawCallback(widget);
      return;
    }

    constexpr int extraSize = 16;
    const Rectangle clippingRectangle = graphicsContext->getClippingRectangle();

    bool drewAny = false;
    int firstXDrawn = 0;
    int lastXDrawn = 0;
    for (Widget* widget : agui::Widget::VisibleChildren(&parent))
      if (widget->shouldRender())
      {
        const Point widgetPosition = Point(absolutePosition, widget->getLocation(), leftPadding, topPadding);
        const Dimension size = widget->getSize();
        const Rectangle widgetRectangle = Rectangle(Point(widgetPosition.x - extraSize, widgetPosition.y - extraSize),
                                                    Dimension(size.width + extraSize * 2, size.height + extraSize * 2));
        if (clippingRectangle.collide(widgetRectangle))
        {
          if (!drewAny)
          {
            firstXDrawn = widgetPosition.x;
            drewAny = true;
          }
          lastXDrawn = widgetRectangle.x + widgetRectangle.width;
          drawCallback(widget);
        }
        else if (drewAny &&
                 widgetRectangle.x >= firstXDrawn &&
                 widgetRectangle.x + widgetRectangle.width <= lastXDrawn)
          break;
      }
  }
}

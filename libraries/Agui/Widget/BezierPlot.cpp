#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include <Agui/Font.hpp>
#include <Agui/Gui.hpp>
#include "Agui/Widget/BezierPlot.hpp"
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

#include <Agui/Widget/Graph.hpp>

namespace agui
{
  GraphStyle BezierPlot::defaultStyle;

  BezierPlot::BezierPlot(const GraphStyle* parentStyle)
    : style(parentStyle ? parentStyle : &Graph::defaultStyle)
  {
    this->update();
  }

  void BezierPlot::paintComponent(const PaintEvent& paintEvent, const Point&)
  {
    //paint graph background6
    paintEvent.graphics()->drawFilledRectangle(this->graphRectangle, this->style.getBackgroundColor());

    Rectangle& sourceRectangle = this->dataRectangle;
    Rectangle& targetRectangle = this->graphRectangle;
    auto remapToTarget = [&](Point point)
    {
      if (this->clampValues)
      {
        while (point.x < sourceRectangle.getLeft())   point.x += sourceRectangle.getWidth();
        while (point.x > sourceRectangle.getRight())  point.x -= sourceRectangle.getWidth();
        while (point.y < sourceRectangle.getTop())    point.y += sourceRectangle.getHeight();
        while (point.y > sourceRectangle.getBottom()) point.y -= sourceRectangle.getHeight();
      }
      return Point(targetRectangle.getLeft() + (point.x - sourceRectangle.getLeft()) * double(targetRectangle.getWidth()) / sourceRectangle.getWidth(),
                   targetRectangle.getTop() + (point.y - sourceRectangle.getTop()) * double(targetRectangle.getHeight()) / sourceRectangle.getHeight());
    };

    const bool isColor = !this->colorPoints.empty();

    if (sourceRectangle.getTop() <= 0 && sourceRectangle.getBottom() >= 0)
      paintEvent.graphics()->drawLine(remapToTarget(Point(0, sourceRectangle.getBottom())),
                                      remapToTarget(Point(0, sourceRectangle.getTop())),
                                      this->style.getGuideLinesColor(), 2);
    if (sourceRectangle.getLeft() <= 0 && sourceRectangle.getRight() >= 0)
      paintEvent.graphics()->drawLine(remapToTarget(Point(sourceRectangle.getLeft(), 0)),
                                      remapToTarget(Point(sourceRectangle.getRight(), 0)),
                                      this->style.getGuideLinesColor(), 2);

    if (isColor)
    {
      for (uint32_t i = 1; i < this->colorPoints.size(); i++)
      {
        paintEvent.graphics()->drawLine(remapToTarget(Point(i, 0)),
                                        remapToTarget(Point(i, 1)),
                                        this->colorPoints.at(i), 2);
      }
    }

    //paint grid lines
    for (int32_t i = 0; i <= 10; i++)
    {
      paintEvent.graphics()->drawLine(remapToTarget(Point(sourceRectangle.getLeft(), sourceRectangle.getTop() + sourceRectangle.getHeight() * i / 10.0f)),
                                      remapToTarget(Point(sourceRectangle.getRight(), sourceRectangle.getTop() + sourceRectangle.getHeight() * i / 10.0f)),
                                      this->style.getGridLinesColor(), 1);
    }

    for (int32_t i = 0; i <= 10; i++)
    {
      paintEvent.graphics()->drawLine(remapToTarget(Point(sourceRectangle.getLeft() + sourceRectangle.getWidth() * i / 10.0f, sourceRectangle.getTop())),
                                      remapToTarget(Point(sourceRectangle.getLeft() + sourceRectangle.getWidth() * i / 10.0f, sourceRectangle.getBottom())),
                                      this->style.getGridLinesColor(), 1);
    }

    if (isColor)
      return;

    for (uint32_t i = 1; i < this->dataPoints.size(); i++)
      paintEvent.graphics()->drawLine(remapToTarget(this->dataPoints.at(i - 1)),
                                      remapToTarget(this->dataPoints.at(i)),
                                      this->style.getLineColors().at(this->colorIndex % this->style.getLineColors().size()), 1);
    paintEvent.graphics()->drawText(targetRectangle.getLeftTop(),
                                    std::to_string(sourceRectangle.getLeft()) + "," + std::to_string(-sourceRectangle.getTop()),
                                    this->style.getGuideLinesColor(), this->style.getFont(), RichTextSetting::Disabled, HorizontalAlign::Left);
    paintEvent.graphics()->drawText(targetRectangle.getBottomLeft() + agui::Point(0, -15),
                                    std::to_string(sourceRectangle.getLeft()) + "," + std::to_string(-sourceRectangle.getBottom()),
                                    this->style.getGuideLinesColor(), this->style.getFont(), RichTextSetting::Disabled, HorizontalAlign::Left);
    paintEvent.graphics()->drawText(targetRectangle.getRightBottom() + agui::Point(0, -15),
                                    std::to_string(sourceRectangle.getRight()) + "," + std::to_string(-sourceRectangle.getBottom()),
                                    this->style.getGuideLinesColor(), this->style.getFont(), RichTextSetting::Disabled, HorizontalAlign::Right);
  }

  void BezierPlot::paintBackground(const PaintEvent& paintEvent, const agui::Point&)
  {
    paintEvent.graphics()->drawFilledRectangle(this->getSizeRectangle(), agui::Color(0.0f, 0.0f, 0.0f));
  }

  void BezierPlot::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->update();
  }

  void BezierPlot::update()
  {
    Rectangle contentRectangle = this->getContentRectangle();
    this->graphRectangle = Rectangle(contentRectangle.getLeft() + this->style.getVerticalLabelsMargin(),
                                     contentRectangle.getTop() + this->style.getGraphTopMargin(),
                                     contentRectangle.getWidth() - (this->style.getVerticalLabelsMargin() + this->style.getGraphRightMargin()),
                                     contentRectangle.getHeight() - (this->style.getHorizontalLabelsMargin() + this->style.getGraphTopMargin()));
  }

}

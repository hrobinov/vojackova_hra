#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Graph.hpp"
#include <Agui/Font.hpp>
#include <Agui/Gui.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

namespace agui
{
  GraphStyle Graph::defaultStyle;

  Graph::Graph(const uint32_t sampleCount,
               const std::function<std::string(float)> verticalLabelFormatter,
               const std::function<std::string(uint32_t)> horizontalLabelFormatter,
               std::function<std::string(float, uint32_t)> tooltipFormater,
               const GraphStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &Graph::defaultStyle)
    , verticalLabelFormatter(verticalLabelFormatter)
    , horizontalLabelFormatter(horizontalLabelFormatter)
    , tooltipFormater(tooltipFormater)
    , sampleCount(sampleCount)
  {
    this->update();
  }

  Graph::~Graph()
  {
    this->removeGraphToolTip();
  }

  //TODO:
  // [?]labels should move up or down as the scale changes
  // [x]or the scale should always show the minimum and maximum value, so the distance between the labels is minimalVerticalLabelSpacing or more
  // [ ]print NO DATA
  // [x]top margin and dont move top label
  // [x]dont paint last line
  // [x]fix lines sometimes dwawing outside the background
  // [x]tooltips
  // [ ]long boolean for formatters, used in tooltips

  // try to hijack tooltip creation request. Or create tooltip as soon as possible?
  // create tooltip at mouse location(it can be created at the point location but there's not much difference). Tooltip should preferably be on the left
  // update tooltip contents when data is changed
  // destroy the tooltip on mouse move. Or when new series is selected?

  void Graph::recreateLabels()
  {
    this->clear();
    this->horizontalLabels.clear();
    this->verticalLabels.clear();

    //adjust the spacing so the last value is always at the end of the graph
    this->adjustedVerticalLabelSpacing = double(this->graphRectangle.getHeight()) /
                                         floor(double(this->graphRectangle.getHeight()) / this->style.getMinimalVerticalLabelSpacing());
    this->adjustedHorizontalLabelSpacing = double(this->graphRectangle.getWidth()) /
                                           floor(double(this->graphRectangle.getWidth()) / this->style.getMinimalHorizontalLabelSpacing());

    if (this->graphRectangle.getHeight() > 0)
    {
      uint32_t loops = floor(double(this->graphRectangle.getHeight()) / this->style.getMinimalVerticalLabelSpacing()) + 1;
      for (uint32_t i = 0; i < loops; i++)
      {
        float absolutePosition = this->graphRectangle.getBottom() - (i * this->adjustedVerticalLabelSpacing);
        Label* label = agui::hold(new Label("0", this->style.getVerticalLabelStyle()));
        this->verticalLabels.emplace_back(label,
                                          (double(this->graphRectangle.getBottom()) - absolutePosition) / this->graphRectangle.getHeight(),
                                          absolutePosition);
        *this << *label;
      }
    }

    if (this->graphRectangle.getWidth() > 0)
    {
      uint32_t loops = floor(double(this->graphRectangle.getWidth()) / this->style.getMinimalHorizontalLabelSpacing()) + 1;
      for (uint32_t i = 0; i < loops; i++)
      {
        float absolutePosition = this->graphRectangle.getRight() - (i * this->adjustedHorizontalLabelSpacing);
        Label* label = agui::hold(new Label("0", this->style.getHorizontalLabelStyle()));
        this->horizontalLabels.emplace_back(label,
                                            (double(this->graphRectangle.getRight()) - absolutePosition) / this->graphRectangle.getWidth(),
                                            absolutePosition);
        *this << *label;
      }
    }

    this->repositionLables();
  }

  void Graph::repositionLables()
  {
    if (this->maximumValue == 0 || this->seriesData.empty())
    {
      for (const AxisLabel& axisLabel : this->verticalLabels)
        axisLabel.label->setRender(false);
      for (const AxisLabel& axisLabel : this->horizontalLabels)
        axisLabel.label->setRender(false);

      return;
    }

    for (const AxisLabel& axisLabel : this->verticalLabels)
    {
      axisLabel.label->setText(this->verticalLabelFormatter(this->minimumValue + axisLabel.normalizedPosition * (this->maximumValue - this->minimumValue)));
      axisLabel.label->resizeToContents();
      axisLabel.label->setLocation(std::max(0, int(this->style.getVerticalLabelsMargin()) - axisLabel.label->getRequiredWidth()),
                                   axisLabel.absolutePosition - axisLabel.label->getRequiredHeight() / 2);
      axisLabel.label->setRender(true);
    }
    assert(this->sampleCount > 1);
    for (const AxisLabel& axisLabel : this->horizontalLabels)
    {
      axisLabel.label->setText(this->horizontalLabelFormatter(std::round(axisLabel.normalizedPosition * (this->sampleCount - 1))));
      axisLabel.label->resizeToContents();
      axisLabel.label->setLocation(axisLabel.absolutePosition - axisLabel.label->getRequiredWidth() / 2, this->graphRectangle.getBottom());
      axisLabel.label->setRender(true);
    }
  }

  void Graph::calculateHighlitedData()
  {
    this->highlightedXIndex = -1;
    this->highlightedDataSeriesIndex = -1;
    this->highlightedXMarkIndex = -1;
    this->highlightedYPosition = -std::numeric_limits<float>::infinity();

    if (this->maximumValue == 0 || this->seriesData.empty())
      return;

    this->repositionLables();

    const float currentHighlightedPreciseXIndex = (this->mouseX - this->graphRectangle.getLeft() + 1) / this->getWidthPerValue();
    const int32_t currentHighlightedXIndex = round(currentHighlightedPreciseXIndex);

    if (currentHighlightedXIndex >= 0 && uint32_t(currentHighlightedXIndex) < this->sampleCount)
    {
      const float currentHighlightedYValue = this->mouseY;
      this->highlightedYPosition = currentHighlightedYValue;

      if (currentHighlightedYValue >= this->graphRectangle.getTop() &&
          currentHighlightedYValue < this->graphRectangle.getTop() + this->graphRectangle.getHeight())
      {
        this->highlightedXIndex = currentHighlightedXIndex;
        this->highlightedDataSeriesIndex = -1;

        const double dataLineHighlightDistance = double(this->style.getDataLineHighlightDistance());
        float minDiff = std::numeric_limits<float>::max();
        //search for the nearest index
        for (int j = 0; j < int(this->seriesData.size()) && j < int(this->style.getLineColors().size()); ++j)
        {
          if (this->seriesData[j].values.size() <= 1)
            continue;
          const int shift = int(this->sampleCount) - int(this->seriesData[j].values.size());
          const int32_t i = this->highlightedXIndex - shift;
          if (i >= 0)
          {
            float diff = fabs(currentHighlightedYValue - this->yValueToPixelPosition(this->seriesData[j].values[i]));
            if (diff < minDiff &&
                diff < dataLineHighlightDistance)
            {
              this->highlightedDataSeriesIndex = j;
              minDiff = diff;
            }
          }
        }

        // see if one of the marks is closer to the cursor
        // translate minDiff from y-axis scaling to x-axis scaling
        minDiff /= ((this->maximumValue - this->minimumValue) / this->graphRectangle.getHeight()) * this->getWidthPerValue();

        for (int j = 0; j < int(this->additionalXMarks.size()); ++j)
        {
          if (this->additionalXMarks[j].toolTipCaption.empty())
            continue;
          float diff = fabs(this->additionalXMarks[j].sampleIndex - currentHighlightedPreciseXIndex);
          if (diff < minDiff &&
              diff < dataLineHighlightDistance / this->getWidthPerValue())
          {
            this->highlightedXIndex = -1;
            this->highlightedDataSeriesIndex = -1;
            this->highlightedXMarkIndex = j;
            minDiff = diff;
          }
        }
      }
    }

    if (this->highlightedDataSeriesIndex != -1 && this->highlightedXIndex != -1)
    {
      const int shift = int(this->sampleCount) - int(this->seriesData[highlightedDataSeriesIndex].values.size());
      const int32_t highlightedXDataIndex = this->highlightedXIndex - shift;
      this->createGraphToolTip();

      this->toolTip->caption = this->seriesData[this->highlightedDataSeriesIndex].label;
      this->toolTip->text = this->tooltipFormater(this->seriesData[this->highlightedDataSeriesIndex].values[highlightedXDataIndex],
                                                     this->sampleCount - 1 - this->highlightedXIndex);
      if (this->tooltipCaptionColor)
        this->toolTip->captionColor = this->tooltipCaptionColor(this->highlightedDataSeriesIndex);
      else
        this->toolTip->captionColor.reset();
      this->toolTip->updateContent();
    }
    else if (this->highlightedXMarkIndex != -1)
    {
      this->createGraphToolTip();

      this->toolTip->caption = this->additionalXMarks[this->highlightedXMarkIndex].toolTipCaption;
      this->toolTip->text = this->additionalXMarks[this->highlightedXMarkIndex].toolTipText;

      this->toolTip->updateContent();
    }
    else
      this->removeGraphToolTip();
  }

  bool Graph::mouseMove(const MouseEvent& mouseEvent)
  {
    this->mouseX = mouseEvent.getPosition().x;
    this->mouseY = mouseEvent.getPosition().y;
    if (this->toolTip)
      this->toolTip->centerOnMouse();
    this->calculateHighlitedData();
    return true;
  }

  bool Graph::mouseLeave(const MouseEvent&)
  {
    this->mouseX = 0;
    this->mouseY = 0;
    this->removeGraphToolTip();
    return true;
  }

  void Graph::flagForDestruction()
  {
    super::flagForDestruction();
    this->removeGraphToolTip();
  }

  void Graph::reapplySubStyles()
  {
    for (auto& label : this->horizontalLabels)
      label.label->style.setParent(this->style.getHorizontalLabelStyle());
    for (auto& label : this->verticalLabels)
      label.label->style.setParent(this->style.getVerticalLabelStyle());
    super::reapplySubStyles();
  }

  void Graph::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->update();
  }

  void Graph::logic(double timeElapsed)
  {
    super::logic(timeElapsed);

    if (this->tooltipUpdateRequested)
    {
      if (this->toolTip)
        this->calculateHighlitedData();
      this->tooltipUpdateRequested = false;
    }
  }

  void Graph::update()
  {
    const Rectangle contentRectangle = this->getContentRectangle();
    const LabelStyle* labelStyle = this->style.getVerticalLabelStyle();
    uint32_t widestVerticalLabel = 0;
    if (this->minimumValue != 0 || this->maximumValue != 0)
      widestVerticalLabel = std::max(Label::getWidthToFit(labelStyle, this->verticalLabelFormatter(this->minimumValue)),
                                     Label::getWidthToFit(labelStyle, this->verticalLabelFormatter(this->maximumValue)));
    const uint32_t verticalLabelsMargin = std::max(this->style.getVerticalLabelsMargin(), widestVerticalLabel);

    this->graphRectangle = Rectangle(contentRectangle.getLeft() + verticalLabelsMargin,
                                     contentRectangle.getTop() + this->style.getGraphTopMargin(),
                                     contentRectangle.getWidth() - (verticalLabelsMargin + this->style.getGraphRightMargin()),
                                     contentRectangle.getHeight() - (this->style.getHorizontalLabelsMargin() + this->style.getGraphTopMargin()));

    this->recreateLabels();
  }

  void Graph::setValues(std::vector<GraphSeries>&& values)
  {
    this->seriesData = std::move(values);

    //we don't use minimum value for now, but some of the logic for it is already implemented.
    this->minimumValue = 0;
    this->maximumValue = 0;
    for (auto& outerVector : this->seriesData)
      for (auto& value : outerVector.values)
      {
        this->minimumValue = std::min(this->minimumValue, value);
        this->maximumValue = std::max(this->maximumValue, value);
      }

    this->repositionLables();
    this->calculateHighlitedData();

    if (this->highlightedDataSeriesManualIndex != -1 &&
        size_t(this->highlightedDataSeriesManualIndex) >= this->seriesData.size())
      this->highlightedDataSeriesManualIndex = -1;
  }

  void Graph::highlightSeries(int32_t i)
  {
    if (i < 0)
      this->highlightedDataSeriesManualIndex = -1;
    if (size_t(i) < this->seriesData.size())
      this->highlightedDataSeriesManualIndex = i;
  }

  void Graph::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    // The grid lines extend to the top and to the right out of the graph area proper.
    const int rightEdge = this->getContentRectangle().getRight();
    const int topEdge = this->getContentRectangle().getTop();

    //paint grid lines
    uint32_t loops = floor(double(this->graphRectangle.getHeight()) / this->style.getMinimalVerticalLabelSpacing());
    for (uint32_t i = 0; i <= loops; i++)
    {
      const float absolutePosition = this->graphRectangle.getBottom() - (i * this->adjustedVerticalLabelSpacing);
      paintEvent.graphics()->drawLine(Point(this->graphRectangle.getLeft(), absolutePosition),
                                      Point(rightEdge, absolutePosition), this->style.getGridLinesColor(), 1);
    }

    loops = floor(double(this->graphRectangle.getWidth()) / this->style.getMinimalHorizontalLabelSpacing());
    for (uint32_t i = 0; i <= loops; i++)
    {
      const float absolutePosition = this->graphRectangle.getRight() - (i * this->adjustedHorizontalLabelSpacing);
      paintEvent.graphics()->drawLine(Point(absolutePosition, topEdge),
                                      Point(absolutePosition, this->graphRectangle.getBottom()), this->style.getGridLinesColor(), 1);
    }

    //paint highlights
    const double widthPerValue = this->getWidthPerValue();
    if (this->seriesData.empty())
      return;

    //paint additional highlights first, under everything else
    for (const AdditionalXMarkData& additionalMark : this->additionalXMarks)
    {
      if (additionalMark.sampleIndex > this->sampleCount)
        continue;
      const float absolutePosition = this->graphRectangle.getLeft() + additionalMark.sampleIndex * widthPerValue;
      paintEvent.graphics()->drawLine(Point(absolutePosition, this->graphRectangle.getTop()),
                                      Point(absolutePosition, this->graphRectangle.getBottom()), additionalMark.color);
    }

    //paint mouse highlight
    if (this->highlightedXIndex != -1)
    {
      const float absolutePosition = this->graphRectangle.getLeft() + (this->highlightedXIndex * widthPerValue);
      paintEvent.graphics()->drawLine(Point(absolutePosition, this->graphRectangle.getTop()),
                                      Point(absolutePosition, this->graphRectangle.getBottom()), this->style.getGuideLinesColor(), 1);
    }
    if (this->mouseY > this->graphRectangle.getTop() && this->mouseY < this->graphRectangle.getBottom())
    {
      paintEvent.graphics()->drawLine(Point(this->graphRectangle.getLeft(), this->mouseY),
                                      Point(this->graphRectangle.getRight(), this->mouseY), this->style.getGuideLinesColor(), 1);
    }

    //paint graph lines
    for (int j = 0; j < int(this->seriesData.size()) && j < int(this->style.getLineColors().size()); ++j)
      this->paintDataSeries(paintEvent, j, false);

    //repaint selected line and circle
    if (this->highlightedDataSeriesIndex != -1 && this->highlightedXIndex != -1)
    {
      const int shift = int(this->sampleCount) - int(this->seriesData[this->highlightedDataSeriesIndex].values.size());
      const int32_t highlightedXDataIndex = this->highlightedXIndex - shift;

      this->paintDataSeries(paintEvent, this->highlightedDataSeriesIndex, true);
      const Point center = Point(this->graphRectangle.getLeft() + (this->highlightedXIndex * widthPerValue),
                                 this->yValueToPixelPosition(this->seriesData[this->highlightedDataSeriesIndex].values[highlightedXDataIndex]));
      paintEvent.graphics()->drawFilledCircle(center, this->style.getSelectionDotRadius(), this->style.getLineColors()[this->highlightedDataSeriesIndex]);
      paintEvent.graphics()->drawFilledCircle(center, Gui::scale*1, this->style.getGuideLinesColor());
      paintEvent.graphics()->drawCircle(center, this->style.getSelectionDotRadius(), this->style.getGuideLinesColor());
    }
    else if (this->highlightedXMarkIndex != -1 && this->highlightedXMarkIndex < int32_t(this->additionalXMarks.size()))
    {
      const float xPosition = this->graphRectangle.getLeft() + this->additionalXMarks[this->highlightedXMarkIndex].sampleIndex * widthPerValue;
      const agui::Color& color = this->additionalXMarks[this->highlightedXMarkIndex].color;

      for (int x = int(xPosition - 1); x <= int(xPosition + 1); ++x)
        paintEvent.graphics()->drawLine(Point(x, this->graphRectangle.getTop()), Point(x, this->graphRectangle.getBottom()), color);

      const Point center = Point{int(xPosition), this->mouseY};
      paintEvent.graphics()->drawFilledCircle(center, this->style.getSelectionDotRadius(), color);
      paintEvent.graphics()->drawFilledCircle(center, Gui::scale * 1, this->style.getGuideLinesColor());
      paintEvent.graphics()->drawCircle(center, this->style.getSelectionDotRadius(), this->style.getGuideLinesColor());
    }

    //highlight override, doesn't have X marker
    if (this->highlightedDataSeriesManualIndex != -1)
      this->paintDataSeries(paintEvent, this->highlightedDataSeriesManualIndex, true);

    //todo: paint tileset

    //todo: paint tooltip (?)

  }

  void Graph::paintBackground(const PaintEvent& event, const agui::Point&)
  {
    event.graphics()->drawFilledRectangle(this->getSizeRectangle(), this->style.getBackgroundColor());
  }

  double Graph::getWidthPerValue() const
  {
    return double(this->graphRectangle.getWidth()) / double(this->sampleCount - 1);
  }

  double Graph::yValueToPixelPosition(const float value) const
  {
    if (this->maximumValue == 0 || this->maximumValue - this->minimumValue == 0)
      return double(this->graphRectangle.getHeight() - 1) + this->graphRectangle.getTop();

    return (1 - double(value - this->minimumValue) / double(this->maximumValue - this->minimumValue)) *
           double(this->graphRectangle.getHeight() - 1) + this->graphRectangle.getTop();
  }

  void Graph::paintDataSeries(const PaintEvent& paintEvent, const int series, const bool highlight)
  {
    if (this->seriesData[series].values.size() <= 1)
      return;
    const int shift = int(this->sampleCount) - int(this->seriesData[series].values.size());

    std::vector<Point> points;
    points.reserve(this->sampleCount - shift + 1);

    for (int i = shift; i < int(this->sampleCount); ++i)
    {
      points.emplace_back(int(i * this->getWidthPerValue()) + this->graphRectangle.getLeft(),
        int(this->yValueToPixelPosition(this->seriesData[series].values[i - shift])));
    }

    //to avoid jitter, this should be using floats
    paintEvent.graphics()->drawPolyline(points, this->style.getLineColors()[series]);

    if (highlight)
    {
      //paint one above and one below
      for (size_t i = 0; i < points.size(); i++)
        points[i].y -= 1;
      paintEvent.graphics()->drawPolyline(points, this->style.getLineColors()[series]);
      for (size_t i = 0; i < points.size(); i++)
        points[i].y += 2;
      paintEvent.graphics()->drawPolyline(points, this->style.getLineColors()[series]);
    }
  }

  void Graph::createGraphToolTip()
  {
    if (!this->toolTip)
      this->toolTip = new ToolTip();
  }

  void Graph::removeGraphToolTip()
  {
    if (!this->toolTip)
      return;
    this->toolTip->flagForDestruction();
    this->toolTip = nullptr;
  }
}

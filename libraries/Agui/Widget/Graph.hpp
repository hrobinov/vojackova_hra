#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/GraphStyle.hpp"
#include <functional>

namespace agui
{
  class Graph final : public Widget
  {
    using super = Widget;
    class AxisLabel
    {
    public:
      AxisLabel(Label* const label, const float normalizedPosition, const float absolutePosition)
        : label(label)
        , normalizedPosition(normalizedPosition)
        , absolutePosition(absolutePosition) {}

      Label* label;
      float normalizedPosition;
      float absolutePosition;
    };
  public:
    class GraphSeries
    {
    public:
      std::string label; //translated label
      std::vector<float> values;
    };
    class AdditionalXMarkData
    {
    public:
      AdditionalXMarkData(uint32_t sampleIndex, agui::Color color, std::string toolTipCaption = "", std::string toolTipText = "")
        : sampleIndex(sampleIndex)
        , color(color)
        , toolTipCaption(std::move(toolTipCaption))
        , toolTipText(std::move(toolTipText))
      {}

      uint32_t sampleIndex;
      agui::Color color;
      std::string toolTipCaption, toolTipText;
    };
    Graph(uint32_t length,
          std::function<std::string(float)> verticalLabelFormatter,
          std::function<std::string(uint32_t)> horizontalLabelFormatter,
          std::function<std::string(float, uint32_t)> tooltipFormater,
          const GraphStyle* parentStyle = nullptr);
    ~Graph();
    virtual Style* getStyle() override { return &this->style; }
    void setValues(std::vector<GraphSeries>&& values);
    // manually highlight series, independent of hover highlight, -1 to clear
    void highlightSeries(int32_t i);
    void forceTooltipUpdate() { this->tooltipUpdateRequested = true; }
  protected:
    virtual void logic(double timeElapsed) override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent&, const agui::Point& absolutePosition) override;
    virtual void onSizeChanged(Dimension) override;
    virtual bool mouseMove(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual void flagForDestruction() override;
    virtual void reapplySubStyles() override;
  private:
    //destroy and recreate labels. Should be used when the size changes and we might end up having a different number of labels
    void recreateLabels();
    //labels change their position based on their content. This should be called every time the data changes.
    void repositionLables();
    // used to recalculated highlightedXIndex and highlightedDataSeriesIndex
    void calculateHighlitedData();
    double getWidthPerValue() const;
    double yValueToPixelPosition(float value) const;
    void paintDataSeries(const PaintEvent& paintEvent, int series, const bool highlight);

    void createGraphToolTip();
    void removeGraphToolTip();
    void update();
  public:
    static GraphStyle defaultStyle;
    GraphStyle style;
    std::function<std::string(float)> verticalLabelFormatter;
    std::function<std::string(uint32_t)> horizontalLabelFormatter;
    std::function<std::string(float, uint32_t)> tooltipFormater;
    std::function<std::optional<agui::Color>(uint32_t)> tooltipCaptionColor;
    //we calculate the min and max once and use it throughout the code
    float maximumValue = 0;
    float minimumValue = 0;
    //additional highlight marks provided from outside
    std::vector<AdditionalXMarkData> additionalXMarks;
  private:
    uint32_t sampleCount;
    std::vector<GraphSeries> seriesData;
    //hold lables so we can change and move them later
    std::vector<AxisLabel> horizontalLabels;
    std::vector<AxisLabel> verticalLabels;
    Rectangle graphRectangle;
    double adjustedVerticalLabelSpacing = 0;
    double adjustedHorizontalLabelSpacing = 0;

    int mouseX = 0, mouseY = 0;
    //the X index the mouse is nearest to
    int32_t highlightedXIndex = -1;
    //the data series index the mouse is closest to on the above X index
    int32_t highlightedDataSeriesIndex = -1;
    // highlighted x-mark index and y-position in case one of those is closest
    int32_t highlightedXMarkIndex = -1;
    float highlightedYPosition = -std::numeric_limits<float>::infinity();
    // manually, independently highlighted series
    int32_t highlightedDataSeriesManualIndex = -1;

    agui::GenericTargeter<ToolTip> toolTip;
    bool tooltipUpdateRequested = false;
  };
}

#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include <Agui/Widget/LabelStyle.hpp>
#include <vector>
#include <memory>

namespace agui
{
  class Graph;

  class GraphStyle : public Style
  {
  private:
    using super = Style;
  public:
    explicit GraphStyle(const GraphStyle* parent = nullptr);
    explicit GraphStyle(Widget* relatedWidget, const GraphStyle* parent = nullptr);
    const GraphStyle* getParent() const { return static_cast<const GraphStyle*>(this->parent); }
    virtual void clear() override;

    const Color& getBackgroundColor() const { return *this->getProperty(&GraphStyle::backgroundColor); }
    void setBackgroundColor(const Color& backgroundColor) { this->backgroundColor.reset(new Color(backgroundColor)); }
    const std::vector<Color>& getLineColors() const { return *this->getProperty(&GraphStyle::lineColors); }
    void setLineColors(const std::vector<Color>& lineColors) { this->lineColors.reset(new std::vector<Color>(lineColors)); }
    double getMinimalHorizontalLabelSpacing() const { return this->getProperty(&GraphStyle::minimalHorizontalLabelSpacing); }
    void setMinimalHorizontalLabelSpacing(double minimalHorizontalLabelSpacing) { this->minimalHorizontalLabelSpacing = minimalHorizontalLabelSpacing; }
    uint32_t getMinimalVerticalLabelSpacing() const { return this->getProperty(&GraphStyle::minimalVerticalLabelSpacing); }
    void setMinimalVerticalLabelSpacing(uint32_t minimalVerticalLabelSpacing) { this->minimalVerticalLabelSpacing = minimalVerticalLabelSpacing; }
    uint32_t getHorizontalLabelsMargin() const { return this->getProperty(&GraphStyle::horizontalLabelsMargin); }
    void setHorizontalLabelsMargin(uint32_t horizontalLabelsMargin) { this->horizontalLabelsMargin = horizontalLabelsMargin; }
    uint32_t getVerticalLabelsMargin() const { return this->getProperty(&GraphStyle::verticalLabelsMargin); }
    void setVerticalLabelsMargin(uint32_t verticalLabelsMargin) { this->verticalLabelsMargin = verticalLabelsMargin; }
    uint32_t getGraphTopMargin() const { return this->getProperty(&GraphStyle::graphTopMargin); }
    void setGraphTopMargin(uint32_t graphTopMargin) { this->graphTopMargin = graphTopMargin; }
    uint32_t getGraphRightMargin() const { return this->getProperty(&GraphStyle::graphRightMargin); }
    void setGraphRightMargin(uint32_t graphRightMargin) { this->graphRightMargin = graphRightMargin; }
    uint32_t getDataLineHighlightDistance() const { return this->getProperty(&GraphStyle::dataLineHighlightDistance); }
    void setDataLineHighlightDistance(uint32_t dataLineHighlightDistance) { this->dataLineHighlightDistance = dataLineHighlightDistance; }
    uint32_t getSelectionDotRadius() const { return this->getProperty(&GraphStyle::selectionDotRadius); }
    void setSelectionDotRadius(uint32_t selectionDotRadius) { this->selectionDotRadius = selectionDotRadius; }
    const Color& getGridLinesColor() const { return *this->getProperty(&GraphStyle::gridLinesColor); }
    void setGridLinesColor(const Color& gridLinesColor) { this->gridLinesColor.reset(new Color(gridLinesColor)); }
    const Color& getGuideLinesColor() const { return *this->getProperty(&GraphStyle::guideLinesColor); }
    void setGuideLinesColor(const Color& guideLinesColor) { this->guideLinesColor.reset(new Color(guideLinesColor)); }
    const LabelStyle* getHorizontalLabelStyle() const { return this->getProperty(&GraphStyle::horizontalLabelStyle); }
    LabelStyle* initHorizontalLabelStyle() { return this->initProperty(&GraphStyle::horizontalLabelStyle); }
    const LabelStyle* getVerticalLabelStyle() const { return this->getProperty(&GraphStyle::verticalLabelStyle); }
    LabelStyle* initVerticalLabelStyle() { return this->initProperty(&GraphStyle::verticalLabelStyle); }
    const Font* getFont() const { return this->getProperty(&GraphStyle::font); }
    void setFont(const Font* font) { this->setProperty(&GraphStyle::font, font); }

  private:
    std::unique_ptr<Color> backgroundColor;
    std::unique_ptr<std::vector<Color>> lineColors;
    std::optional<double> minimalHorizontalLabelSpacing;
    std::optional<uint32_t> minimalVerticalLabelSpacing;
    std::optional<uint32_t> horizontalLabelsMargin;
    std::optional<uint32_t> verticalLabelsMargin;
    std::optional<uint32_t> graphTopMargin;
    std::optional<uint32_t> graphRightMargin;
    std::optional<uint32_t> dataLineHighlightDistance;
    std::optional<uint32_t> selectionDotRadius;
    std::unique_ptr<Color> gridLinesColor;
    std::unique_ptr<Color> guideLinesColor;
    std::unique_ptr<LabelStyle> horizontalLabelStyle;
    std::unique_ptr<LabelStyle> verticalLabelStyle;
    const Font* font = nullptr;
  };
}

#include "Agui/Widget/Graph.hpp"
#include "Agui/Widget/BezierPlot.hpp"
#include "Agui/Widget/GraphStyle.hpp"
#include <Agui/Widget/LabelStyle.hpp>

namespace agui
{
  GraphStyle::GraphStyle(const GraphStyle* parent)
    : GraphStyle(nullptr, parent)
  {}

  GraphStyle::GraphStyle(Widget* relatedWidget, const GraphStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void GraphStyle::clear()
  {
    super::clear();
    this->backgroundColor.reset();
    this->lineColors.reset();
    this->minimalHorizontalLabelSpacing.reset();
    this->minimalVerticalLabelSpacing.reset();
    this->horizontalLabelsMargin.reset();
    this->verticalLabelsMargin.reset();
    this->graphTopMargin.reset();
    this->graphRightMargin.reset();
    this->dataLineHighlightDistance.reset();
    this->selectionDotRadius.reset();
    this->gridLinesColor.reset();
    this->guideLinesColor.reset();
    this->horizontalLabelStyle.reset();
    this->verticalLabelStyle.reset();
    this->font = nullptr;
  }
}

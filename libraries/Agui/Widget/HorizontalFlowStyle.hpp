#pragma once
#include "Agui/Style.hpp"

namespace agui
{
  class HorizontalFlow;

  class HorizontalFlowStyle : public Style
  {
    using super = Style;
  public:
    explicit HorizontalFlowStyle(const HorizontalFlowStyle* parent = nullptr);
    explicit HorizontalFlowStyle(HorizontalFlow* relatedWidget, const HorizontalFlowStyle* parent = nullptr);
    const HorizontalFlowStyle* getParent() const { return static_cast<const HorizontalFlowStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    int getHorizontalSpacing() const { return this->getProperty(&HorizontalFlowStyle::horizontalSpacing); }
    void setHorizontalSpacing(int horizontalSpacing) { this->horizontalSpacing = horizontalSpacing; }

  private:
    std::optional<int> horizontalSpacing;
  };
}

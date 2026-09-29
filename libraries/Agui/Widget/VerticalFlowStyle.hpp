#pragma once
#include "Agui/Style.hpp"

namespace agui
{
  class VerticalFlow;

  class VerticalFlowStyle : public Style
  {
    using super = Style;
  public:
    explicit VerticalFlowStyle(const VerticalFlowStyle* parent = nullptr);
    explicit VerticalFlowStyle(VerticalFlow* relatedWidget, const VerticalFlowStyle* parent = nullptr);
    const VerticalFlowStyle* getParent() const { return static_cast<const VerticalFlowStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    int getVerticalSpacing() const { return this->getProperty(&VerticalFlowStyle::verticalSpacing); }
    void setVerticalSpacing(int verticalSpacing) { this->verticalSpacing = verticalSpacing; }

  private:
    std::optional<int> verticalSpacing;
  };
}

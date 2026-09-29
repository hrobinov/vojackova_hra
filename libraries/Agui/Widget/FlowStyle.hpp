#pragma once
#include "Agui/Style.hpp"

namespace agui
{
  class Layout;

  class FlowStyle : public Style
  {
    using super = Style;
  public:
    explicit FlowStyle(const FlowStyle* parent = nullptr);
    explicit FlowStyle(Layout* relatedWidget, const FlowStyle* parent = nullptr);
    const FlowStyle* getParent() const { return static_cast<const FlowStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    uint32_t getMaxOnRow() const { return this->getProperty(&FlowStyle::maxOnRow); }
    void setMaxOnRow(uint32_t maxOnRow) { this->maxOnRow = maxOnRow;}
    int getHorizontalSpacing() const { return this->getProperty(&FlowStyle::horizontalSpacing); }
    void setHorizontalSpacing(int horizontalSpacing) { this->horizontalSpacing = horizontalSpacing; }
    int getVerticalSpacing() const { return this->getProperty(&FlowStyle::verticalSpacing); }
    void setVerticalSpacing(int verticalSpacing) { this->verticalSpacing = verticalSpacing; }

  private:
    std::optional<uint32_t> maxOnRow;
    std::optional<int> horizontalSpacing;
    std::optional<int> verticalSpacing;
  };
}

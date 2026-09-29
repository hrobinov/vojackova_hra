#pragma once
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include <memory>

namespace agui
{
  class EmptyWidget;

  class EmptyWidgetStyle : public Style
  {
    using super = Style;
  public:
    explicit EmptyWidgetStyle(const EmptyWidgetStyle* parent = nullptr);
    explicit EmptyWidgetStyle(EmptyWidget* relatedWidget, const EmptyWidgetStyle* parent = nullptr);
    const EmptyWidgetStyle* getParent() const { return static_cast<const EmptyWidgetStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const ElementImageSet* getGraphicalSet() const { return this->getProperty(&EmptyWidgetStyle::graphicalSet); }
    void initGraphicalSet(ElementImageSet* graphicalSet) { this->graphicalSet.reset(new ElementImageSet(*graphicalSet)); }

  private:
    std::unique_ptr<ElementImageSet> graphicalSet;
  };
}

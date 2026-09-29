#pragma once
#include "Agui/BorderImageSet.hpp"
#include "Agui/Style.hpp"
#include <memory>

namespace agui
{
  class Line;
  class LineStyle : public Style
  {
    using super = Style;
  public:
    explicit LineStyle(const LineStyle* parent = nullptr);
    explicit LineStyle(Line* relatedWidget, const LineStyle* parent = nullptr);
    const LineStyle* getParent() const { return static_cast<const LineStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const BorderImageSet* getBorder() const;
    void setBorder(const BorderImageSet& borderImageSet);

  private:
    std::unique_ptr<BorderImageSet> borderImageSet;
  };
}

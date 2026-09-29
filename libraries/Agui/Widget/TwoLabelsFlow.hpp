#pragma once
#include "Agui/Layout.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/FlowStyle.hpp"

namespace agui
{

  /** This is very specialized layout to handle two labels in all circumstances
   * Case 1 - enough space:
   *  |left| |right|
   * Case 2 - not enough space
   *  |left|
   *    |long right|
   * Case 3 - even less space:
   *  |left|
   *    |very very |
   *    |right that|
   *    |needs to  |
   *    |be split  |
   */
  class TwoLabelsFlow final : public Layout
  {
    using super = Layout;
  public:
    TwoLabelsFlow(const LabelStyle* leftStype, const LabelStyle* rightStyle);
    ~TwoLabelsFlow() = default;
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override;
    virtual Style* getStyle() override { return &this->style; }

    FlowStyle style;
    Label left, right;
  };
}

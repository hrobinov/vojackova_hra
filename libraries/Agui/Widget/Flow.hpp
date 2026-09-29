#pragma once
#include "Agui/Layout.hpp"
#include "Agui/AlignmentEnum.hpp"
#include "Agui/AreaAlignmentEnum.hpp"
#include "Agui/Widget/FlowStyle.hpp"
namespace agui
{
  class Flow : public Layout
  {
    using super = Layout;
  public:
    Flow(const FlowStyle* parent = &Flow::defaultStyle);
    virtual ~Flow();

  protected:
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override;
    void finishRow(std::vector<Widget*>& currentRow, int& highestWidget, int& x, int& y);
    virtual int maximumHorizontalSquashSize() const override;

  public:
    virtual Style* getStyle() override { return &this->style; }
    virtual void setMinOnRow(uint32_t min);
    virtual int getContentsHeight() const;
    virtual int getContentsWidth() const;
    virtual void resizeToContents() override;

    static FlowStyle defaultStyle;

    FlowStyle style;
  private:
    uint32_t contentsHeight = 0;
    uint32_t minOnRow = 0;
  };
}

#pragma once
#include "Agui/Style.hpp"
#include "Agui/Widget/FrameStyle.hpp"
#include "Agui/Widget/HorizontalFlowStyle.hpp"
#include "Agui/Widget/TableStyle.hpp"

#include <memory>

namespace agui
{
  class TabbedPane;

  class TabbedPaneStyle : public Style
  {
    using super = Style;
  public:
    explicit TabbedPaneStyle(const TabbedPaneStyle* parent = nullptr);
    explicit TabbedPaneStyle(TabbedPane* relatedWidget, const TabbedPaneStyle* parent = nullptr);
    const TabbedPaneStyle* getParent() const { return static_cast<const TabbedPaneStyle*>(this->parent); }
    virtual void clear() override;

    int32_t getVerticalSpacing() const { return this->getProperty(&TabbedPaneStyle::verticalSpacing); }
    void setVerticalSpacing(int32_t verticalSpacing) { this->setProperty(&TabbedPaneStyle::verticalSpacing, verticalSpacing); }
    const FrameStyle* getContentFrame() const { return this->getProperty(&TabbedPaneStyle::contentFrame); }
    FrameStyle* initContentFrame() { return this->initProperty(&TabbedPaneStyle::contentFrame); }
    const TableStyle* getTabContainerStyle() const { return this->getProperty(&TabbedPaneStyle::tabContainerStyle); }
    TableStyle* initTabContainerStyle() { return this->initProperty(&TabbedPaneStyle::tabContainerStyle); }

  private:
    std::optional<int32_t> verticalSpacing;
    std::unique_ptr<FrameStyle> contentFrame;
    std::unique_ptr<TableStyle> tabContainerStyle;
  };
}

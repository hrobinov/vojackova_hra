#include "Agui/Widget/TabbedPaneStyle.hpp"
#include "Agui/Widget/TabbedPane.hpp"

namespace agui
{
  TabbedPaneStyle::TabbedPaneStyle(const TabbedPaneStyle* parent)
    : TabbedPaneStyle(nullptr, parent)
  {}

  TabbedPaneStyle::TabbedPaneStyle(TabbedPane* relatedWidget, const TabbedPaneStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictions(*this);
  }

  void TabbedPaneStyle::clear()
  {
    super::clear();
    this->verticalSpacing.reset();
    this->contentFrame.reset();
    this->tabContainerStyle.reset();
  }
}

#include "Agui/Widget.hpp"
#include "Agui/Widget/ScrollBarStyle.hpp"

namespace agui
{
  ScrollBarStyle::ScrollBarStyle(const ScrollBarStyle* parent)
    : ScrollBarStyle(nullptr, parent)
  {}

  ScrollBarStyle::ScrollBarStyle(Widget* relatedWidget, const ScrollBarStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }
  void ScrollBarStyle::clear()
  {
    super::clear();
    this->thumbButtonStyle.reset();
    this->backgroundGraphicalSet.reset();
  }
}

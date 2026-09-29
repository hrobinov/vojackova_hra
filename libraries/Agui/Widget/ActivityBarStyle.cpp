#include "Agui/Widget/ActivityBar.hpp"
#include "Agui/Widget/ActivityBarStyle.hpp"

namespace agui
{
  ActivityBarStyle::ActivityBarStyle(const ActivityBarStyle* parent)
    : ActivityBarStyle(nullptr, parent)
  {}

  void ActivityBarStyle::clear()
  {
    super::clear();
    this->speed.reset();
    this->barWidth.reset();
    this->color.reset();
    this->bar = nullptr;
    this->barSizeRatio.reset();
    this->background = nullptr;
  }

  agui::ActivityBarStyle::ActivityBarStyle(ActivityBar* relatedWidget, const ActivityBarStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }
}

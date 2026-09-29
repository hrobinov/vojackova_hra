#include "Agui/Widget/ListBoxStyle.hpp"
#include "Agui/Widget/ListBox.hpp"
#include <cassert>

namespace agui
{
  ListBoxStyle::ListBoxStyle(const ListBoxStyle* parent)
    : ListBoxStyle(nullptr, parent)
  {}

  ListBoxStyle::ListBoxStyle(ListBox* relatedWidget, const ListBoxStyle* parent /*= nullptr*/)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void ListBoxStyle::clear()
  {
    super::clear();
    this->itemStyle.reset();
    this->scrollPaneStyle.reset();
  }
}

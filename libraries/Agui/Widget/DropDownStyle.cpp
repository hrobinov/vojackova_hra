#include "Agui/Widget/DropDown.hpp"
#include "Agui/Widget/DropDownStyle.hpp"
#include "Agui/Widget/ListBox.hpp"

namespace agui
{
  DropDownStyle::DropDownStyle(const DropDownStyle* parent)
    : DropDownStyle(nullptr, parent)
  {}

  DropDownStyle::DropDownStyle(DropDown* relatedWidget, const DropDownStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void DropDownStyle::clear()
  {
    super::clear();
    this->buttonStyle.reset();
    this->icon = nullptr;
    this->openedSound = nullptr;
    this->listBoxStyle.reset();
    this->selectorAndTitleSpacing.reset();
  }

  void DropDownStyle::setOpenedSound(std::nullptr_t)
  {
    this->openedSound = &EmptySound::instance;
  }
}

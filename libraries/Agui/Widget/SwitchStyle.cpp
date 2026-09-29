#include "Agui/Widget/SwitchStyle.hpp"
#include "Agui/Widget/Switch.hpp"

namespace agui
{
  SwitchStyle::SwitchStyle(const SwitchStyle* parent)
    : SwitchStyle(nullptr, parent)
  {}

  SwitchStyle::SwitchStyle(Switch* relatedWidget, const SwitchStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void SwitchStyle::clear()
  {
    super::clear();
    this->leftButtonPosition.reset();
    this->middleButtonPosition.reset();
    this->rightButtonPosition.reset();
    this->backgroundDefault = nullptr;
    this->backgroundHover = nullptr;
    this->backgroundDisabled = nullptr;
    this->activeLabelStyle.reset();
    this->inactiveLabelStyle.reset();
    this->buttonStyle.reset();
  }
}
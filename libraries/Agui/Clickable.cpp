#include "Agui/Clickable.hpp"
#include "Agui/Sound.hpp"
#include <Agui/StyleWithClickableGraphicalSet.hpp>

namespace agui
{
  void Clickable::changeClickState(Clickable::ClickState state)
  {
    if (this->state == state)
      return;
    this->state = state;
    this->callRecursively([hovered = this->state != ClickState::DEFAULT](agui::Widget* widget){ widget->setParentHovered(hovered); });
    this->onClickStateChanged();
  }

  Clickable::ClickState Clickable::getClickState() const
  {
    return this->state;
  }

  Clickable::ClickState Clickable::getClickStateForRendering() const
  {
    switch (this->state)
    {
      case ClickState::DEFAULT:
        if (this->renderAsHovered)
          return ClickState::HOVERED;
        [[fallthrough]];
      case ClickState::HOVERED:
      case ClickState::CLICKED:
        break;
    }
    return this->getClickState();
  }

  void Clickable::modifyClickState()
  {
    if (this->clickStateLocked)
      return;
    if (!this->isEnabled())
    {
      this->changeClickState(ClickState::DEFAULT);
      return;
    }
    if (this->mouseIsDown && this->mouseIsInside)
      this->changeClickState(ClickState::CLICKED);
    else if (this->mouseIsDown && !this->mouseIsInside)
      this->changeClickState(this->getMouseLeaveState());
    else if (!this->mouseIsDown && this->mouseIsInside)
      this->changeClickState(ClickState::HOVERED);
    else
      this->changeClickState(ClickState::DEFAULT);
  }

  void Clickable::focusGained(TabbedIn tabbedIn)
  {
    Widget::focusGained(tabbedIn);
    this->clickStateLocked = false;
    this->modifyClickState();
  }

  void Clickable::focusLost()
  {
    Widget::focusLost();
    this->clickStateLocked = false;
    this->mouseIsDown = false;
    this->modifyClickState();
  }

  bool Clickable::mouseEnter(const MouseEvent& mouseEvent)
  {
    super::mouseEnter(mouseEvent);
    this->mouseIsInside = true;
    this->modifyClickState();
    return true;
  }

  bool Clickable::mouseLeave(const MouseEvent& mouseEvent)
  {
    super::mouseLeave(mouseEvent);
    this->mouseIsInside = false;
    this->modifyClickState();
    return true;
  }

  bool Clickable::mouseDown(const MouseEvent&)
  {
    this->mouseIsDown = true;
    if (this->shouldPlaySound())
      if (auto* style = dynamic_cast<const StyleWithClickableGraphicalSet*>(this->getStyle()))
        if (const Sound* sound = style->getLeftClickSound())
          sound->play(1);
    this->modifyClickState();
    return true;
  }

  bool Clickable::mouseUp(const MouseEvent& mouseEvent)
  {
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return false;
    this->mouseIsDown = false;
    this->modifyClickState();
    return true;
  }

  void Clickable::setMouseLeaveState(Clickable::ClickState state)
  {
    this->mouseLeaveState = state;
  }

  Clickable::ClickState Clickable::getMouseLeaveState() const
  {
    return this->mouseLeaveState;
  }

  void Clickable::setClickState(Clickable::ClickState state)
  {
    this->changeClickState(state);
  }

  void Clickable::resetMouseState()
  {
    this->state = Clickable::defaultState;
    this->mouseIsDown = false;
    this->clickStateLocked = false;
    this->mouseLeaveState = Clickable::defaultState;
    this->mouseIsInside = false;
    this->callRecursively([](agui::Widget* widget){ widget->setParentHovered(false); });
  }

  bool Clickable::isMouseInside() const
  {
    return this->mouseIsInside;
  }

  Widget& Clickable::setEnabled(bool enabled)
  {
    if (this->isEnabled() == enabled)
      return *this;
    this->resetMouseState();
    return super::setEnabled(enabled);
  }
}

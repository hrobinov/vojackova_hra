#include <Agui/EventDispatchHelper.hpp>
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Switch.hpp"
#include <cassert>
#include <Agui/Input.hpp>

namespace agui
{
  SwitchStyle Switch::defaultStyle;

  Switch::Switch(SwitchStyle* parentStyle)
    : style(this, parentStyle)
  {}

  void Switch::paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    (void)paintEvent;
    // Background
    const Image* background = nullptr;
    if (this->isEnabled())
      switch (this->getClickStateForRendering())
      {
        case ClickState::DEFAULT: background = this->style.getBackgroundDefault(); break;
        case ClickState::HOVERED: [[fallthrough]];
        case ClickState::CLICKED: background = this->style.getBackgroundHover(); break;
      }
    else
      background = this->style.getBackgroundDisabled();
    paintEvent.graphics()->drawScaledImage(background,
                                           Point(0, 0),
                                           Dimension(this->getContentWidth(), this->getContentHeight()));
    // The button
    int position = 0; // horizontal offset from the left
    switch (this->state)
    {
      case SwitchState::Left: position = this->style.getLeftButtonPosition(); break;
      case SwitchState::Right: position = this->style.getRightButtonPosition(); break;
      case SwitchState::None: position = this->style.getMiddleButtonPosition(); break;
    }
    int buttonHeight = this->style.getButtonStyle()->getMinimalHeight();
    const Rectangle positionRectangle = { position,
                                          (this->getContentHeight() - buttonHeight) / 2 + 1, // center the button vertically
                                          this->style.getButtonStyle()->getMinimalWidth(),
                                          buttonHeight };

    if (this->isEnabled())
      switch (this->getClickStateForRendering())
      {
        case ClickState::HOVERED: this->style.getButtonStyle()->getHoveredGraphicalSet()->base.draw(paintEvent, positionRectangle, absolutePosition); break;
        case ClickState::CLICKED: this->style.getButtonStyle()->getClickedGraphicalSet()->base.draw(paintEvent, positionRectangle, absolutePosition); break;
        case ClickState::DEFAULT: this->style.getButtonStyle()->getDefaultGraphicalSet()->base.draw(paintEvent, positionRectangle, absolutePosition); break;
      }
    else
      this->style.getButtonStyle()->getDisabledGraphicalSet()->base.draw(paintEvent, positionRectangle, absolutePosition);
  }

  void Switch::resizeToContents()
  {
    this->setContentSize(this->style.getMinimalWidth(), this->style.getMinimalHeight());
  }

  void Switch::setState(SwitchState state)
  {
    assert(allowNoneState || state != SwitchState::None);
    this->state = state;
  }

  bool Switch::mouseDown(const MouseEvent& mouseEvent)
  {
    super::mouseDown(mouseEvent);


    SwitchState newState;

    if (this->allowNoneState)
    {
      if (this->getGui()->input->getInputMethod() == Input::PlayerInputMethod::GameController)
      {
        if (this->state == SwitchState::Left)
          newState = SwitchState::None;
        else if (this->state == SwitchState::None)
          newState = SwitchState::Right;
        else
          newState = SwitchState::Left;
      }
      else
      {
        Point p = mouseEvent.getPosition();
        if (p.x >= 2 * this->getContentWidth() / 3)
          newState = SwitchState::Right;
        else if (p.x >= this->getContentWidth() / 3)
          newState = SwitchState::None;
        else
          newState = SwitchState::Left;
      }
    }
    else
      newState = this->state == SwitchState::Left ? SwitchState::Right : SwitchState::Left;

    bool changed = newState != this->state;
    this->state = newState;
    if (changed)
    {
      this->dispatchSwitchToggle();
      if (const Sound* sound = this->style.getButtonStyle()->getLeftClickSound())
        sound->play(1);
    }
    return true;
  }

  void Switch::onSwitchToggle(GenericTargetable* owner, std::function<void()> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnSwitchToggle);
    this->actionListeners.back().onSwitchToggle = std::move(callback);
  }

  void Switch::dispatchSwitchToggle()
  {
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnSwitchToggle))
      listener.onSwitchToggle();
  }
}

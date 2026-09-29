#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Button.hpp"
#include "Agui/Widget/Label.hpp"
#include <stdexcept>

namespace agui
{
  ButtonStyle Button::defaultStyle;

  agui::Button::Button(const ButtonStyle* parentStyle)
    : style(this, parentStyle)
  {}

  bool Button::shouldClipRendering() const
  {
    const ElementImageSet* currentImageSet = this->getCurrentImageSet();
    return currentImageSet->shadow.type == ElementImageSet::Layer::Type::None &&
           currentImageSet->glow.type == ElementImageSet::Layer::Type::None;
  }

  bool Button::isToggleButton() const
  {
    return this->isButtonToggleButton;
  }

  bool Button::isToggled() const
  {
    return this->toggled;
  }

  void Button::setToggleButton(bool toggleButton)
  {
    this->isButtonToggleButton = toggleButton;
    if (!toggleButton)
      this->toggled = false;
    this->modifyClickState();
  }

  agui::Widget& Button::startToggle(bool toggled)
  {
    this->setToggleButton(true);
    this->setToggleState(toggled);
    return *this;
  }

  void Button::handleToggleClick(bool leftClick)
  {
    if (!this->toggled || this->isAutoUntoggling())
    {
      this->modifyIsToggled(!this->toggled);
      this->modifyClickState();
    }
    this->dispatchToggle(leftClick);
  }

  void Button::modifyClickState()
  {
    if (this->isDoingKeyAction)
      return;
    super::modifyClickState();
  }

  void Button::changeClickState(Clickable::ClickState state)
  {
    super::changeClickState(state);
    this->updateLabelFontColor();
  }

  void Button::modifyIsToggled(bool toggled)
  {
    this->toggled = toggled;
  }

  void Button::focusGained(TabbedIn tabbedIn)
  {
    super::focusGained(tabbedIn);
    this->isDoingKeyAction = false;
    this->clickStateLocked = false;
    this->modifyClickState();
  }

  void Button::focusLost()
  {
    super::focusLost();
    this->isDoingKeyAction = false;
    this->clickStateLocked = false;
    this->modifyClickState();
  }

  bool Button::mouseDown(const MouseEvent& mouseEvent)
  {
    super::mouseDown(mouseEvent);
    return true;
  }

  bool Button::mouseClick(const MouseEvent& mouseEvent)
  {
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return false;
    if (this->isToggleButton())
      this->handleToggleClick(mouseEvent.getButton() == MouseButton::LEFT);
    return true;
  }

  bool Button::keyUp(const KeyEvent& keyEvent)
  {
    if (!this->isDoingKeyAction)
      return false;
    this->isDoingKeyAction = false;
    this->clickStateLocked = false;
    if (keyEvent.getKey() == KEY_ENTER)
    {
      this->handleToggleClick();
      this->modifyClickState();
      this->dispatchConfirm(keyEvent);
      return true;
    }
    return false;
  }

  int Button::currentVerticalOffset() const
  {
    if (this->getClickStateForRendering() == ClickState::CLICKED || this->isToggled())
      return this->style.getClickedVerticalOffset();
    return 0;
  }

  void Button::addRemark(agui::Label* remark)
  {
    remark->style.setFontColor(this->getFontColor());
    this->addPrivateChild(remark);
  }

  void Button::addRemark(const std::string& remarkText)
  {
    this->addRemark(&agui::label(remarkText));
  }

  bool Button::keyDown(const KeyEvent& keyEvent)
  {
    if (keyEvent.getKey() != KEY_ENTER)
      return super::keyDown(keyEvent);
    this->isDoingKeyAction = true;
    this->clickStateLocked = true;
    this->changeClickState(ClickState::CLICKED);
    return true;
  }

  void Button::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->shadowView)
      this->getCurrentImageSet()->shadow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void Button::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->getCurrentImageSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void Button::paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->glowView)
      this->getCurrentImageSet()->glow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void Button::setToggleState(bool toggled)
  {
    if (this->toggled != toggled)
    {
      this->modifyIsToggled(toggled);
      this->modifyClickState();
    }
  }

  void Button::resetState()
  {
    this->isDoingKeyAction = false;
    this->isButtonToggleButton = false;
    this->toggled = false;
    this->autoUntoggle = true;
    this->resetMouseState();
  }

  const ElementImageSet* Button::getCurrentImageSet() const
  {
    if (this->isEnabled())
      switch (this->getClickStateForRendering())
      {
        case ClickState::HOVERED:
          if (this->toggled)
            if (this->autoUntoggle)
              return this->style.getSelectedHoveredGraphicalSet();
            else
              return this->style.getSelectedGraphicalSet(); // no reaction to hover when toggled and autotoggle is off, as the click should do nothing
          return this->style.getHoveredGraphicalSet();
        case ClickState::CLICKED:
          if (this->toggled && this->autoUntoggle)
            return this->style.getSelectedClickedGraphicalSet();
          return this->style.getClickedGraphicalSet();
        case ClickState::DEFAULT: return this->toggled ? this->style.getSelectedGraphicalSet() : this->style.getDefaultGraphicalSet();
        default: throw std::logic_error("Unknown click state");
      }
    else
      return this->style.getDisabledGraphicalSet();
  }

  Color Button::getFontColor() const
  {
    if (this->isEnabled())
      switch (this->state)
      {
        case ClickState::DEFAULT: return this->toggled ? this->style.getSelectedFontColor() : this->style.getDefaultFontColor();
        case ClickState::HOVERED: return this->toggled ? this->style.getSelectedHoveredFontColor() : this->style.getHoveredFontColor();
        case ClickState::CLICKED: return this->toggled ? this->style.getSelectedClickedFontColor() : this->style.getClickedFontColor();
        default: throw std::logic_error("Unknown click state");
      }
    else
      return this->style.getDisabledFontColor();
  }

  void Button::updateLabelFontColor()
  {
    if (this->getPrivateChildCount() != 0)
      if (Label* label = dynamic_cast<Label*>(this->getPrivateChildAt(0u)))
        label->style.setFontColor(this->getFontColor());
  }

  Widget& Button::setEnabled(bool enabled)
  {
    Widget& result = Clickable::setEnabled(enabled);
    this->updateLabelFontColor();
    return result;
  }

  void Button::setButtonState(Button::ClickState state)
  {
    this->changeClickState(state);
  }

  void Button::setAutoUntoggle(bool untoggle)
  {
    this->autoUntoggle = untoggle;
  }

  bool Button::isAutoUntoggling() const
  {
    return this->autoUntoggle;
  }

  const ElementImageSet* Button::getBorderImageSet() const
  {
    return this->style.getDefaultGraphicalSet();
  }
}

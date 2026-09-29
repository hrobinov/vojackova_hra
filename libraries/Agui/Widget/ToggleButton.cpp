#include <Agui/EventDispatchHelper.hpp>
#include "Agui/Widget/ToggleButton.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Font.hpp"
#include "Agui/Image.hpp"
#include <algorithm>

namespace agui
{
  ToggleButton::ToggleButton()
  {
    this->positionButton();
    this->mouseButtonFilter = MouseButton::LEFT;
  }

  ToggleButton::ToggleButton(std::string&& text)
  {
    this->setText(std::move(text));
  }

  ToggleButton::~ToggleButton()
  {}

  Dimension ToggleButton::getButtonSize() const
  {
      Dimension dimension = this->getDimension();
      return Dimension(dimension.width * Gui::scale, dimension.height * Gui::scale);
  }

  void ToggleButton::nextCheckState()
  {
    if (!this->isEnabled())
      return;
    switch (this->getCheckedState())
    {
      case CheckedState::INTERMEDIATE:
      case CheckedState::UNCHECKED: this->changeCheckedState(CheckedState::CHECKED); break;
      case CheckedState::CHECKED: this->changeCheckedState(CheckedState::UNCHECKED); break;
    }
    this->dispatchCheckChange();
    if (this->isChecked())
      this->dispatchCheck();
  }

  bool ToggleButton::dispatchCheckChange()
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnCheckChange);
    for (Listener& listener : helper)
      listener.onCheckChange(this->isChecked());
    return bool(helper);
  }

  bool ToggleButton::dispatchCheck()
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnCheck);
    for (Listener& listener : helper)
      listener.onCheck();
    return bool(helper);
  }

  ToggleButton::CheckedState ToggleButton::getCheckedState() const
  {
    return this->checkedState;
  }

  void ToggleButton::changeCheckedState(CheckedState state)
  {
    this->checkedState = state;
  }

  void ToggleButton::resizeCaption()
  {
    Rectangle contentRectangle = this->getContentRectangle();
    int x = this->getButtonSize().width + this->getMiddleGap();
    this->wordWrapRect = Rectangle(x,
                                   0,
                                   contentRectangle.width - x,
                                   contentRectangle.height);
  }

  void ToggleButton::positionButton()
  {
    this->buttonPosition = this->createAlignedPosition(agui::AreaAlign::LeftMiddle, this->getContentSizeAsRectangle(), this->getButtonSize());
    this->buttonRect = Rectangle(this->buttonPosition, this->getButtonSize());
  }

  void ToggleButton::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->getBackgroundGraphicalSet()->base.draw(paintEvent, this->getButtonRectangle(), absolutePosition);
  }

  void ToggleButton::paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->glowView)
      this->getBackgroundGraphicalSet()->glow.draw(paintEvent, this->getButtonRectangle(), absolutePosition);
  }

  void ToggleButton::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->shadowView)
      this->getBackgroundGraphicalSet()->shadow.draw(paintEvent, this->getButtonRectangle(), absolutePosition);
  }

  void ToggleButton::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    Rectangle rectangle = this->getButtonRectangle();
    this->drawItem(paintEvent, rectangle);
    this->drawText(paintEvent);
  }

  void ToggleButton::onCheckChange(GenericTargetable* owner, std::function<void(bool)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnCheckChange);
    this->actionListeners.back().onCheckChange = std::move(callback);
  }

  void ToggleButton::onCheckChange(GenericTargetable* owner, std::function<void()> callback)
  {
    return this->onCheckChange(owner, [callback = std::move(callback)](bool) { callback(); });
  }

  void ToggleButton::onCheck(GenericTargetable* owner, std::function<void()> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnCheck);
    this->actionListeners.back().onCheck = std::move(callback);
  }

  void ToggleButton::drawText(const PaintEvent& paintEvent) const
  {
    if (!this->trimmedLine.empty())
      ResizableText::drawTextArea(paintEvent.graphics(),
                                  this->getFont(),
                                  this->wordWrapRect,
                                  this->getFontColor(),
                                  this->trimmedLine,
                                  this->getHorizontalAlign(),
                                  this->getVerticalAlign(),
                                  RichTextSetting::Enabled,
                                  ResizableText::Ellipsis::True);
  }

  void ToggleButton::resizeToContents()
  {
    this->positionButton();

    int sizeX, sizeY;
    if (this->getText().length() == 0)
    {
      sizeX = this->getButtonSize().width;
      sizeY = this->getButtonSize().height;
    }
    else
    {
      sizeX = this->getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled);
      sizeY = this->getFont()->getLineHeight();

      if (this->getButtonSize().height > sizeY)
        sizeY = this->getButtonSize().height;

      sizeX += this->getButtonSize().width + this->getMiddleGap();
    }

    // specifically asking for StretchRule::On as it is the only case when I wait for stretching from outside,
    // unlike when the rule is set to StretchRule::StretchAndExpand.
    if (this->getStyle()->isHorizontallyStretchable() == StretchRule::On)
      sizeX = 0;

    if (this->getStyle()->isVerticallySquashable() == StretchRule::On)
      sizeY = 0;

    this->setSize(sizeX + this->getHorizontalPaddings(), sizeY + this->getVerticalPaddings());
  }

  void ToggleButton::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->positionButton();
    this->resizeCaption();
  }

  void ToggleButton::setChecked(bool checked)
  {
    this->changeCheckedState(checked ? CheckedState::CHECKED : CheckedState::UNCHECKED);
  }

  void ToggleButton::setCheckedState(CheckedState state)
  {
    this->changeCheckedState(state);
  }

  bool ToggleButton::isChecked() const
  {
    return this->checkedState == CheckedState::CHECKED;
  }

  void ToggleButton::setText(const std::string& text)
  {
    this->setText(std::string(text));
  }

  void ToggleButton::setText(std::string&& text)
  {
    // multiline check/radio boxes are not supported now, but if we wanted to do it, more changes would have to be done
    // mainly the wrapping of the trimmedLine string in the setSize method.
    this->trimmedLine = text;
    Widget::setText(std::move(text));
  }

  bool ToggleButton::mouseDown(const MouseEvent& mouseEvent)
  {
    super::mouseDown(mouseEvent);
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return false;
    if (this->isFireClickOnMouseDown())
      if (this->getClickState() == ClickState::CLICKED)
        this->nextCheckState();
    return true;
  }

  bool ToggleButton::mouseUp(const MouseEvent& mouseEvent)
  {
    super::mouseUp(mouseEvent);
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return false;
    if (!this->isFireClickOnMouseDown())
      if (this->getClickState() == ClickState::HOVERED)
        this->nextCheckState();
    return true;
  }

  bool ToggleButton::keyDown(const KeyEvent& keyEvent)
  {
    if (keyEvent.getKey() != KEY_SPACE && keyEvent.getKey() != KEY_ENTER)
      return false;
    this->isDoingKeyAction = true;
    this->setClickState(ClickState::CLICKED);
    return true;
  }

  bool ToggleButton::keyUp(const KeyEvent& keyEvent)
  {
    if (!this->isDoingKeyAction)
      return false;
    if (keyEvent.getKey() != KEY_SPACE && keyEvent.getKey() != KEY_ENTER)
      return false;
    this->isDoingKeyAction = false;
    this->setClickState(ClickState::DEFAULT);
    this->nextCheckState();
    return true;
  }

  const Point& ToggleButton::getButtonPosition() const
  {
    return this->buttonPosition;
  }

  const Rectangle& ToggleButton::getButtonRectangle() const
  {
    return this->buttonRect;
  }

  int ToggleButton::getMiddleGap() const
  {
    return this->getTextPadding() * Gui::scale;
  }

  const Rectangle& ToggleButton::getWordWrapRect() const
  {
    return this->wordWrapRect;
  }
}

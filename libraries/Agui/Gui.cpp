#include "Agui/Gui.hpp"
#include "Agui/TopContainer.hpp"
#include "Agui/Input.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Widget/ListBox.hpp"
#include <Agui/ImeCompositionInfo.hpp>
#include <Agui/TextInputInfo.hpp>
#include <Agui/ScopedSetter.hpp>
#include <algorithm>
#include <cassert>

namespace agui
{
  double Gui::scale = 1.0;
  double Gui::displayDensity = 1.0;
  Gui* Gui::instance = nullptr;

  void Gui::destroyFlaggedWidgets()
  {
    Widget::widgetsToDestroy.destroyWidgets();
  }

  void Gui::addToolTip(ToolTip* toolTip)
  {
    this->tooltips.emplace_back(toolTip);
    this->tooltipToBeAddedAtTheEndOfLogic = toolTip;
  }

  Gui::~Gui()
  {
    Widget::widgetsToDestroy.destroyWidgets();
    delete baseWidget;
    assert(Gui::instance == this);
    Gui::instance = nullptr;
  }

  bool Gui::shouldHoverGui() const
  {
    return this->instantTooltip || this->timeUntilNextGuiHover <= this->input->getTime();
  }

  bool Gui::shouldHoverEntity() const
  {
    return this->instantTooltip || this->timeUntilNextEntityHover <= this->input->getTime();
  }

  void Gui::checkBringClickedWindowToFront()
  {
    if (!this->bringWindowToTopOnClick)
      return;

    if (!this->widgetUnderMouse)
      return;

    const TopContainer* top = this->getTop();
    Widget* widget = *this->widgetUnderMouse;

    do
    {
      Widget* parent = widget->getParent();
      if (parent == top)
      {
        if (this->getWidgetOnTop() != widget)
          if (dynamic_cast<agui::Window*>(widget))
            this->requestBringWidgetToFront(widget);
        break;
      }
      widget = parent;
    } while (widget);
  }

  void Gui::setInstantTooltip(bool instantTooltip)
  {
    if (this->instantTooltip == instantTooltip)
      return;
    this->instantTooltip = instantTooltip;
    if (!this->instantTooltip)
    {
      if (this->currentTooltip &&
          this->currentTooltip->activatedByInstantToolTip &&
          (!this->currentTooltip->isEntityToolTip && this->getGuiHoverInterval() == -1 ||
           this->currentTooltip->isEntityToolTip && this->getEntityHoverInterval() == -1))
        this->currentTooltip->flagForDestruction();

      if (this->tooltipToBeAddedAtTheEndOfLogic && this->tooltipToBeAddedAtTheEndOfLogic->activatedByInstantToolTip)
      {
        this->tooltipToBeAddedAtTheEndOfLogic->flagForDestruction();
        this->tooltipToBeAddedAtTheEndOfLogic.clear();
      }
    }
    else
      this->forceHover();
  }

  bool Gui::isScreenHeightSmall(int32_t heightThreshold)
  {
    return (this->getGraphicsContext()->getDisplaySize().height / Gui::scale < heightThreshold);
  }

  bool Gui::isScreenWidthSmall(int32_t widthThreshold)
  {
    return (this->getGraphicsContext()->getDisplaySize().width / Gui::scale < widthThreshold);
  }

  void Gui::startDragging(Widget* widget)
  {
    assert(widget != this->getTop());
    this->controlWithLock = widget;
    this->commitToMouseDrag = widget && !widget->isRequireMinimumDragDistance();
    if (!this->commitToMouseDrag)
      this->mousePositionAtStartOfDrag = this->currentMousePosition;
  }

  Gui::Gui()
    : baseWidget(new TopContainer(this))
    , emptyMouse(MouseEvent::Type::MOUSE_DOWN, MouseButton::NONE, 0, 0, 0, 0, false, false, false)
  {
    assert(Gui::instance == nullptr);
    Gui::instance = this;
  }

  void Gui::handleMouseAxes(const MouseInput& mouse)
  {
    // handle the mouse move and mouse wheel events

    // obtain mouse data
    this->handleMouseAxes(this->createMouseEvent(mouse));
  }

  void Gui::handleMouseAxes(MouseEvent mouseEvent)
  {
    if (this->currentMousePosition != mouseEvent.getPosition())
      this->resetHoverTime();

    // if the mouse wheel has changed, it is a mouse wheel event
    if (mouseEvent.getMouseWheelChange() != 0)
      if (this->widgetUnderMouse)
      {
        // prevent non modal event
        if (this->focusManager.getModalWidget() && !this->widgetIsModalChild(*this->widgetUnderMouse) && !*this->controlWithLock)
          return;

        MouseEvent relative = this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent);

        switch (mouseEvent.getEvent())
        {
          case MouseEvent::Type::MOUSE_WHEEL_UP:
            this->widgetUnderMouse->dispatchMouseWheelUp(relative);
            break;
          case MouseEvent::Type::MOUSE_WHEEL_DOWN:
            this->widgetUnderMouse->dispatchMouseWheelDown(relative);
            break;
          case MouseEvent::Type::MOUSE_WHEEL_LEFT:
            this->widgetUnderMouse->dispatchMouseWheelLeft(relative);
            break;
          case MouseEvent::Type::MOUSE_WHEEL_RIGHT:
            this->widgetUnderMouse->dispatchMouseWheelRight(relative);
            break;
          default:
            break;
        }
        return;
      }

    if (this->input->getInputMethod() == Input::PlayerInputMethod::KeyboardAndMouse)
    {
      if (mouseEvent.getEvent() == MouseEvent::Type::MOUSE_LEAVE)
      {
        this->mouseInWindow = false;
        if (this->controlWithLock)
        {
          this->controlWithLock->dispatchMouseUp(this->convertMouseEventToRelative(*this->controlWithLock, mouseEvent));
          if (this->controlWithLock && this->widgetUnderMouse != this->controlWithLock)
            this->controlWithLock->removeToolTipWidget();
          this->forceReleaseControlWithLock();
        }
      }
      else if (mouseEvent.getEvent() == MouseEvent::Type::MOUSE_ENTER)
        this->mouseInWindow = true;
    }

    this->currentMousePosition = mouseEvent.getPosition();
    this->orderToUpdateWidgetUnderMouse();

    /* the locked control is valid when the mouse is down during a mouse move in order to prevent
     other widgets from receiving events until mouse up */
    if (!this->controlWithLock)
    {
      if (this->widgetUnderMouse &&
          (!this->focusManager.getModalWidget() || this->widgetIsModalChild(*this->widgetUnderMouse)))
        this->widgetUnderMouse->dispatchMouseMove(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
    }
    else // send a mouse dragged to the locked control
    {
      if (this->previousWidgetUnderMouse != this->widgetUnderMouse)
      {
        // invalidate the last hover control
        this->lastHoveredControl = nullptr;

        // prevent non modal event
        if (this->focusManager.getModalWidget() && !this->widgetIsModalChild(*this->widgetUnderMouse) && !this->controlWithLock)
          return;
      }

      // prevent non modal event
      if (this->focusManager.getModalWidget() && !this->widgetIsModalChild(*this->widgetUnderMouse) && !this->controlWithLock)
        return;

      if (!this->commitToMouseDrag)
      {
        const Point mouseDelta = mouseEvent.getPosition() - this->mousePositionAtStartOfDrag;
        const int dragDistanceThresholdPx = this->getDragDistanceThresholdPx();
        this->commitToMouseDrag = abs(mouseDelta.x) >= dragDistanceThresholdPx || abs(mouseDelta.y) >= dragDistanceThresholdPx;
      }

      if (this->commitToMouseDrag &&
        this->controlWithLock &&
        (!this->controlWithLock->dragOnlyByLeftMouseButton() || mouseEvent.getButton() == MouseButton::LEFT))
        this->controlWithLock->dispatchMouseDrag(this->convertMouseEventToRelative(*this->controlWithLock, mouseEvent));
    }
  }

  void Gui::handleMouseDown(const MouseInput& mouse)
  {
    // if there is a locked control, leave
    if (*this->controlWithLock && mouse.button != this->lastMouseButton)
      return;

    // obtain mouse data
    MouseEvent mouseEvent = this->createMouseEvent(mouse);
    this->handleMouseDown(mouseEvent);
  }

  void Gui::handleMouseDown(const MouseEvent& mouseEvent)
  {
    this->setMouseButtonDown(mouseEvent.getButton());

    this->widgetUnderMouse = this->recursiveGetWidgetUnderMouse(mouseEvent.getPosition());
    Widget* widgetUnderMouseBackup = *this->widgetUnderMouse;
    // invalidate hovering
    this->resetHoverTime();

    // prevent hovering on the widget
    this->lastHoveredControl = widgetUnderMouseBackup;

    // tell modal that a mouse down outside of itself has occurred
    if (this->focusManager.getModalWidget() && !this->widgetIsModalChild(*this->widgetUnderMouse))
    {
      if (GenericTargeter<Widget> widget = this->focusManager.getModalWidget())
      {
        widget->dispatchModalMouseDown(this->convertMouseEventToRelative(*widget, mouseEvent));
        if (!widget || widget != this->focusManager.getModalWidget())
          this->clearWidgetUnderMouse();
      }
      return;
    }

    this->checkBringClickedWindowToFront();

    if (this->widgetUnderMouse)
    {
      this->lastMouseDownControl = *this->widgetUnderMouse;
      if (*this->widgetUnderMouse != this->focusManager.getFocusedWidget())
        // set focus
        if (!this->controlWithLock)
          this->focusManager.setFocusedWidget(*this->widgetUnderMouse);
      // send a mouse down
      if (this->controlWithLock && *this->widgetUnderMouse != *this->controlWithLock)
        return;

      if (this->widgetUnderMouse)
        this->widgetUnderMouse->dispatchMouseDown(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
    }

    // lock it for dragging purposes
    if (!*this->controlWithLock)
    {
      if (this->widgetUnderMouse->isDragEnabled())
        this->startDragging(*this->widgetUnderMouse);
      this->mouseUpControl = *this->widgetUnderMouse;
    }

    if (widgetUnderMouseBackup != *this->widgetUnderMouse && this->widgetUnderMouse)
      this->widgetUnderMouse->dispatchMouseDown(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
  }

  void Gui::handleMouseUp(const MouseInput& mouse)
  {
    // handles the mouse up and click events
    if (mouse.button != this->lastMouseButton)
      return;

    this->handleMouseUp(this->createMouseEvent(mouse));
  }

  void Gui::handleMouseUp(MouseEvent mouseEvent)
  {
    // handles the mouse up and click events
    if (mouseEvent.getButton() != this->lastMouseButton)
      return;

    // invalidate the mouse button
    this->setMouseButtonDown(MouseButton::NONE);

    this->resetHoverTime();

    this->widgetUnderMouse = this->recursiveGetWidgetUnderMouse(mouseEvent.getPosition());

    // prevent non modal event
    if (this->focusManager.getModalWidget() && !this->widgetIsModalChild(*this->widgetUnderMouse) && !this->controlWithLock)
      return;

    if (this->widgetUnderMouse)
    {
      // send a click event if the widget hasn't changed since mouse down
      if (this->lastMouseDownControl == this->widgetUnderMouse &&
          this->widgetUnderMouse == this->controlWithLock &&
          !this->widgetUnderMouse->isFireClickOnMouseDown()) // in this case, click was already fired when processing mouse down event
        this->widgetUnderMouse->dispatchClick(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent.copyWithNewType(MouseEvent::Type::MOUSE_CLICK)));

      // send mouse up
      GenericTargeter<Widget> destinationWidget = *this->mouseUpControl;
      if (!this->mouseUpControl)
        destinationWidget = *this->widgetUnderMouse;
      if (*this->controlWithLock && this->controlWithLock)
        destinationWidget = *this->controlWithLock;
      ListBox* listBox = dynamic_cast<ListBox*>(*destinationWidget);
      if (!this->canDoubleClick ||
          *destinationWidget != *this->widgetOfLastClick ||
          listBox && listBox->getSelectedIndex() != this->listBoxItemIndexOfLastClick)
      {
        this->canDoubleClick = true;
        this->widgetOfLastClick = destinationWidget;
        if (listBox)
          this->listBoxItemIndexOfLastClick = listBox->getSelectedIndex();
        this->resetDoubleClickTime();
      }
      else
      {
        this->canDoubleClick = false;
        if (destinationWidget)
          destinationWidget->dispatchDoubleClick(this->convertMouseEventToRelative(*destinationWidget, mouseEvent));
      }

      if (destinationWidget)
        destinationWidget->dispatchMouseUp(this->convertMouseEventToRelative(*destinationWidget, mouseEvent));
    }

    // unlock the control
    if (this->widgetUnderMouse != this->controlWithLock && this->controlWithLock && this->widgetUnderMouse)
    {
      // lock may be zero if the initially clicked widget did not have dragging enabled; in which case, the widget has already
      // received a MouseEvent and should not receive another one
      this->setCursor(this->widgetUnderMouse->getEnterCursor());
      mouseEvent.previous = *this->previousWidgetUnderMouse;
      this->widgetUnderMouse->dispatchMouseEnter(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
      mouseEvent.previous = nullptr;

      if (this->controlWithLock)
        this->controlWithLock->dispatchMouseLeave(this->convertMouseEventToRelative(*this->controlWithLock, mouseEvent));
    }

    if (this->controlWithLock)
    {
      if (this->widgetUnderMouse != this->controlWithLock)
        this->controlWithLock->removeToolTipWidget();
      this->forceReleaseControlWithLock();
    }
  }

  Widget* Gui::recursiveGetWidgetUnderMouse(Point position)
  {
    if (Widget* underMouse = this->baseWidget->getWidgetUnderMouse(position,
                                                                   agui::Point(0, 0),
                                                                   this->baseWidget->getAbsoluteRectangle(),
                                                                   this->styleView ? TransparentValue::No : TransparentValue::Yes)) // If we are in style debug we want to able to select the transparent widget.
      return underMouse;

    return this->baseWidget; // Nullptr under widget is historically to mean "something modal blocking everything else", so we return the top widget.
  }

  void Gui::resetHoverTime()
  {
    this->resetGuiTooltipHoverTime();
    this->resetEntityTooltipHoverTime();
  }

  void Gui::resetGuiTooltipHoverTime()
  {
    this->timeUntilNextGuiHover = this->guiTooltipHoverInterval == -1 ? std::numeric_limits<float>::max() :  this->input->getTime() + this->guiTooltipHoverInterval;
  }

  void Gui::resetEntityTooltipHoverTime()
  {
    this->timeUntilNextEntityHover = this->entityTooltipHoverInterval == -1 ? std::numeric_limits<float>::max() :  this->input->getTime() + this->entityTooltipHoverInterval;
  }

  void Gui::clearTooltips()
  {
    while (!this->tooltips.empty())
    {
      if (this->tooltips.back())
        this->tooltips.back()->flagForDestruction();
      this->tooltips.pop_back();
    }
  }

  double Gui::getGuiHoverInterval() const
  {
    return this->guiTooltipHoverInterval;
  }

  double Gui::getEntityHoverInterval() const
  {
    return this->entityTooltipHoverInterval;
  }

  void Gui::handleKeyDown(const KeyboardInput& keyboard)
  {
    this->setKeyEvent(keyboard, false);
    this->handleKeyDown(this->keyEvent);
  }

  void Gui::handleKeyDown(const KeyEvent& keyEvent)
  {
    if (&this->keyEvent != &keyEvent)
      this->keyEvent = keyEvent;

    if (this->handleTabbing())
      return;
    if (this->focusManager.getFocusedWidget())
    {
      this->focusManager.getFocusedWidget()->keyDown(keyEvent);
      if (this->focusManager.getFocusedWidget())
        this->focusManager.getFocusedWidget()->_dispatchKeyboardListenerEvent(KeyEvent::KEY_DOWN, this->keyEvent);
    }
  }

  void Gui::handleKeyUp(const KeyEvent& keyEvent)
  {
    if (&this->keyEvent != &keyEvent)
      this->keyEvent = keyEvent;

    if (this->focusManager.getFocusedWidget())
    {
      this->focusManager.getFocusedWidget()->keyUp(this->keyEvent);
      if (this->focusManager.getFocusedWidget())
        this->focusManager.getFocusedWidget()->_dispatchKeyboardListenerEvent(KeyEvent::KEY_UP, this->keyEvent);
    }
  }

  void Gui::setKeyEvent(const KeyboardInput& keyboard, bool handled)
  {
    this->keyEvent = KeyEvent(keyboard.key,
                              keyboard.extKey,
                              keyboard.keyCode,
                              keyboard.scanCode,
                              keyboard.unichar,
                              keyboard.timeStamp,
                              keyboard.isAlt,
                              keyboard.isControl,
                              keyboard.isShift,
                              keyboard.isMeta);
    if (handled)
      this->keyEvent.consume();
  }

  void Gui::handleKeyUp(const KeyboardInput& keyboard)
  {
    this->setKeyEvent(keyboard, false);
    this->handleKeyUp(this->keyEvent);
  }

  void Gui::handleKeyRepeat(const KeyboardInput& keyboard)
  {
    this->setKeyEvent(keyboard, false);
    if (handleTabbing())
      return;
    if (this->focusManager.getFocusedWidget())
    {
      this->focusManager.getFocusedWidget()->keyRepeat(keyEvent);
      if (this->focusManager.getFocusedWidget())
        this->focusManager.getFocusedWidget()->_dispatchKeyboardListenerEvent(KeyEvent::KEY_REPEAT, keyEvent);
    }
  }

  Widget* Gui::getWidgetUnderMouse()
  {
    if (this->updateWidgetUnderMouse)
      this->orderToUpdateWidgetUnderMouse();
    return *this->widgetUnderMouse;
  }

  void Gui::setWidgetUnderMouse(Widget* widget)
  {
    this->widgetUnderMouse = widget;
  }

  MouseButton Gui::getMouseButtonDown() const
  {
    return this->lastMouseButton;
  }

  void Gui::setMouseButtonDown(MouseButton button)
  {
    this->lastMouseButton = button;
  }

  MouseEvent Gui::createMouseEvent(const MouseInput& mouse)
  {
    MouseButton button = mouse.button;
    // create mouse data
    if (this->controlWithLock && mouse.button == MouseButton::NONE)
      button = this->lastMouseButton;
    return MouseEvent(Point(mouse.x, mouse.y),
                      mouse.wheel,
                      button,
                      mouse.type,
                      mouse.timeStamp,
                      mouse.isAlt,
                      mouse.isControl,
                      mouse.isShift);
  }

  void Gui::setLastMouseDownControl(Widget* control)
  {
    this->lastMouseDownControl = control;
  }

  const Widget* Gui::getLastMouseDownControl() const
  {
    return *this->lastMouseDownControl;
  }

  Widget* Gui::getLastMouseDownControl()
  {
    return *this->lastMouseDownControl;
  }

  void Gui::handleHover()
  {
    if (*this->controlWithLock != nullptr)
      return;

    // dispatches a hover event
    if (this->shouldHoverGui())
    {
      this->resetGuiTooltipHoverTime();
      this->getWidgetUnderMouse();
      if (*this->widgetUnderMouse != *this->lastHoveredControl)
      {
        this->lastHoveredControl = *this->widgetUnderMouse;
        if (*this->widgetUnderMouse && this->widgetUnderMouse &&
            (this->focusManager.getModalWidget() && this->widgetIsModalChild(*this->widgetUnderMouse) ||
             !this->focusManager.getModalWidget()))
          this->widgetUnderMouse->dispatchMouseHover(this->convertMouseEventToRelative(*this->widgetUnderMouse, MouseEvent(this->currentMousePosition)));
      }
    }
  }

  void Gui::flagForForceHover()
  {
    this->wantsForceHover = true;
  }

  bool Gui::flaggedForForceHover() const
  {
    return this->wantsForceHover;
  }

  void Gui::forceHover()
  {
    this->wantsForceHover = false;
    this->timeUntilNextGuiHover = 0;
    this->lastHoveredControl = nullptr;
    this->getTop()->processTriggersToResize(); // Ensure everything has the correct size so any tooltip-based-off-size works correctly
    this->handleHover();
  }

  void Gui::setGuiTooltipHoverInterval(double time)
  {
    this->guiTooltipHoverInterval = time;
  }

  void Gui::setEntityTooltipHoverInterval(double time)
  {
    this->entityTooltipHoverInterval = time;
  }

  void Gui::resetDoubleClickTime()
  {
    this->doubleClickExpireTime = this->input->getTime() + this->doubleClickInterval;
  }

  void Gui::handleDoubleClick()
  {
    // prevent non modal event
    if (this->canDoubleClick)
      if (this->input->getTime() > this->doubleClickExpireTime)
        this->canDoubleClick = false;
  }

  double Gui::getDoubleClickInterval() const
  {
    return this->doubleClickInterval;
  }

  void Gui::setDoubleClickInterval(double time)
  {
    this->doubleClickInterval = time;
  }

  bool Gui::widgetIsModalChild(Widget* widget) const
  {
    if (!this->focusManager.getModalWidget())
      return false;

    if (widget == this->focusManager.getModalWidget())
      return true;

    Widget* currentParent = widget;
    if (widget)
      while (currentParent)
      {
        if (currentParent == this->focusManager.getModalWidget())
          return true;
        if (!currentParent->getParent())
          return false;
        currentParent = currentParent->getParent();
      }

    return false;
  }

  Widget* Gui::getFocusedWidget()
  {
    return this->focusManager.getFocusedWidget();
  }

  Widget* Gui::getWidgetOnTop()
  {
    if (!this->baseWidget->getChildren().empty())
      return this->baseWidget->getChildren().back();
    if (!this->baseWidget->getPrivateChildren().empty())
      return this->baseWidget->getPrivateChildren().back();
    return nullptr;
  }

  Widget* Gui::getNonTooltipWidgetOnTop()
  {
    auto firstNonTooltip =  [](std::vector<Widget*>& widgets) -> Widget*
    {
      auto it = widgets.rbegin();
      auto end = widgets.rend();
      for (; it != end; ++it)
        if (!dynamic_cast<ToolTip*>(*it))
          return *it;
      return nullptr;
    };

    if (Widget* widget = firstNonTooltip(this->baseWidget->getChildren()))
      return widget;
    if (Widget* widget = firstNonTooltip(this->baseWidget->getPrivateChildren()))
      return widget;
    return nullptr;
  }

  bool Gui::recursiveFocusNext(Widget* target, Widget* focused)
  {
    if (target && (!target->isVisible() || !target->shouldRender()))
      return false;

    if (focused == nullptr && !this->passedFocus)
      this->passedFocus = true;

    if (!target)
      return false;

    if (target->shouldSkipForFocusNext())
      return false;

    if (target != focused &&
        target->isFocusable() &&
        target->isTabable() &&
        target->isEnabled() &&
        this->passedFocus)
    {
      target->focus(TabbedIn::True);
      return true;
    }

    if (target->isFocused() && target != focused)
      return true;

    if (target == focused)
      this->passedFocus = true;

    if (target->isEnabled())
    {
      for (Widget* widget : target->getPrivateChildren())
        if (this->recursiveFocusNext(widget, focused))
          return true;
      for (Widget* widget : *target)
        if (this->recursiveFocusNext(widget, focused))
          return true;
    }
    return false;
  }

  void Gui::focusNextTabableWidget()
  {
    this->passedFocus = false;
    if (!this->recursiveFocusNext(this->baseWidget, this->focusManager.getFocusedWidget()))
    {
      this->passedFocus = true;
      this->recursiveFocusNext(this->baseWidget, this->focusManager.getFocusedWidget());
    }
  }

  bool Gui::recursiveFocusPrevious(Widget* target,
                                   Widget* focused)
  {
    if (target && (!target->isVisible() || !target->shouldRender()))
      return false;

    if (!focused && !this->passedFocus)
      this->passedFocus = true;

    if (!target)
      return false;

    if (target->shouldSkipForFocusNext())
      return false;

    if (target->isEnabled())
    {
      {
        auto it = target->getPrivateChildren().rbegin();
        auto end = target->getPrivateChildren().rend();
        for (; it != end; ++it)
          if (this->recursiveFocusPrevious(*it, focused))
            return true;
      }
      {
        auto it = target->getChildren().rbegin();
        auto end = target->getChildren().rend();
        for (; it != end; ++it)
          if (this->recursiveFocusPrevious(*it, focused))
            return true;
      }
    }

    if (target != focused &&
        target->isFocusable() &&
        target->isTabable() &&
        target->isEnabled() &&
        this->passedFocus)
      {
        // tabbed panes should not be previously tabbed
        // to avoid circular dependency
        target->focus(TabbedIn::True);
        return true;
      }

    if (target->isFocused() && target != focused)
      return true;

    if (target == focused)
      this->passedFocus = true;

    return false;
  }

  void Gui::focusPreviousTabableWidget()
  {
    this->passedFocus = false;
    if (!this->recursiveFocusPrevious(this->baseWidget, this->focusManager.getFocusedWidget()))
    {
      this->passedFocus = true;
      this->recursiveFocusPrevious(this->baseWidget, this->focusManager.getFocusedWidget());
    }
  }

  bool Gui::isTabbingEnabled() const
  {
    return tabbingEnabled;
  }

  void Gui::setTabbingEnabled(bool tabbing)
  {
    this->tabbingEnabled = tabbing;
  }

  void Gui::widgetLocationChanged()
  {
    this->updateWidgetUnderMouse = true;
  }

  void Gui::removeWidget(Widget* widget)
  {
    this->widgetLocationChanged();

    if (widget == *this->widgetUnderMouse)
      this->clearWidgetUnderMouse();

    if (widget == *this->lastMouseDownControl)
      this->lastMouseDownControl = nullptr;

    if (widget == *this->previousWidgetUnderMouse)
      this->previousWidgetUnderMouse = nullptr;

    if (widget == this->focusManager.getFocusedWidget())
      this->focusManager.clearFocusedWidget();

    if (widget == *this->controlWithLock)
      this->forceReleaseControlWithLock();

    if (widget == *this->lastHoveredControl)
      this->lastHoveredControl = nullptr;

    this->focusManager.releaseModalFocus(widget);
  }

  MouseEvent Gui::convertMouseEventToRelative(Widget* source, const MouseEvent& mouseEvent)
  {
    agui::Point absolutePosition = source->getAbsolutePosition();
    return MouseEvent(Point(mouseEvent.getPosition().x - absolutePosition.x,
                            mouseEvent.getPosition().y - absolutePosition.y),
                      mouseEvent.getMouseWheelChange(),
                      mouseEvent.getButton(),
                      mouseEvent.getEvent(),
                      mouseEvent.getTimeStamp(),
                      mouseEvent.alt(),
                      mouseEvent.control(),
                      mouseEvent.shift(),
                      source,
                      mouseEvent.previous);
  }

  void Gui::logic(bool regularUpdate)
  {
    // Ensure everything is where it should be so any position-related actions interact with the correct widgets
    if (this->getTop()->processTriggersToResize())
      this->anchorAnchoredWidgets();
    this->input->pollInput();
    Widget::widgetsToDestroy.destroyWidgets();
    this->dispatchKeyboardEvents();
    Widget::widgetsToDestroy.destroyWidgets();
    this->dispatchMouseEvents();
    this->currentTime = this->input->getTime();
    Widget::widgetsToDestroy.destroyWidgets();
    if (regularUpdate)
    {
      this->recursiveDoLogic(this->baseWidget);
      this->clearInvalidTooltips();
    }
    if (this->updateWidgetUnderMouse)
      this->orderToUpdateWidgetUnderMouse();
    while (!this->callAfterResizeIsSettledActions.empty())
    {
      std::vector<GenericTargeter<Widget>> targets(std::move(this->callAfterResizeIsSettledActions));
      for (GenericTargeter<Widget>& item : targets)
        if (item)
          item->afterResizeIsSettledAction();
    }
    if (*this->widgetToBringToFront)
    {
      this->focusManager.checkThatModalFocusedWigetIsOnTop();
      this->widgetToBringToFront->bringToFront();
      this->widgetToBringToFront = nullptr;
    }
    Widget::widgetsToDestroy.destroyWidgets();
    this->handleTimedEvents();
    if (this->currentTooltip && this->currentTooltip->doTooltipLogic())
      this->currentTooltip->bringToFront();
    Widget::widgetsToDestroy.destroyWidgets();
    if (this->wantsForceHover)
      this->forceHover();
    if (this->tooltipToBeAddedAtTheEndOfLogic)
    {
      this->currentTooltip = this->tooltipToBeAddedAtTheEndOfLogic;
      this->add(*this->tooltipToBeAddedAtTheEndOfLogic);
      this->tooltipToBeAddedAtTheEndOfLogic.clear();
    }
    if (regularUpdate)
    {
      this->updateFlyingTexts();
      this->updateTextInputInfo();
      this->updateImeComposition();
    }
  }

  void Gui::recursiveDoLogic(Widget* baseWidget)
  {
    if (baseWidget->isFlaggedForDestruction())
      return;
    baseWidget->logic(currentTime);
    if (baseWidget->shouldSkipChildLogic())
      return;

    {
#ifdef DEBUG
      ScopedSetter guard(baseWidget->privateChildrenBeingIterated, true);
#endif

      auto& privateChildren = baseWidget->getPrivateChildren();
      for (auto it = privateChildren.data(), end = it + privateChildren.size(); it != end; ++it)
        this->recursiveDoLogic(*it);
    }
    {
#ifdef DEBUG
      ScopedSetter guard(baseWidget->childrenBeingIterated, true);
#endif

      auto& children = baseWidget->getChildren();
      for (auto it = children.data(), end = it + children.size(); it != end; ++it)
        this->recursiveDoLogic(*it);
    }
  }

  void Gui::clearInvalidTooltips()
  {
    for (size_t i = 0; i < this->tooltips.size();)
    {
      ToolTip* tooltip = *this->tooltips[i];
      if (tooltip && !tooltip->isValid())
        tooltip->flagForDestruction();

      if (!this->tooltips[i])
      {
        if (i + 1 != this->tooltips.size())
          this->tooltips[i] = this->tooltips.back();
        this->tooltips.pop_back();
      }
      else
        ++i;
    }
  }

  void Gui::setGraphics(Graphics* context)
  {
    this->graphicsContext = context;
    if (this->input)
      this->baseWidget->setSize(this->graphicsContext->getDisplaySize().width, this->graphicsContext->getDisplaySize().height);
  }

  void Gui::setInput(Input* input)
  {
    this->input = input;
    this->timeUntilNextGuiHover = this->guiTooltipHoverInterval == -1 ? std::numeric_limits<float>::max() :  this->input->getTime() + this->guiTooltipHoverInterval;
    this->timeUntilNextEntityHover = this->entityTooltipHoverInterval == -1 ? std::numeric_limits<float>::max() :  this->input->getTime() + this->entityTooltipHoverInterval;
    if (this->graphicsContext)
      this->baseWidget->setSize(graphicsContext->getDisplaySize().width, graphicsContext->getDisplaySize().height);
  }

  void Gui::setSize(int width, int height)
  {
    this->baseWidget->setSize(width, height);
  }

  void Gui::render()
  {
    if (!this->shouldRender)
      return;
    this->graphicsContext->clearClippingStack();
    this->graphicsContext->pushClippingRect(this->baseWidget, Rectangle(Point(0, 0), this->baseWidget->getSize()), true);

    if (this->baseWidget->isVisible())
      this->baseWidget->recursivePaintChildren(true, this->graphicsContext, this->baseWidget->getAbsolutePosition());

    this->graphicsContext->clearClippingStack();
    this->graphicsContext->setOffset(Point(0, 0));
    this->graphicsContext->pushClippingRect(this->baseWidget, Rectangle(Point(0, 0), this->baseWidget->getSize()), true);
    for (const GuiFlyingText& flyingText : this->flyingTexts)
      flyingText.paint(this->graphicsContext);
    if (this->styleView)
      if (Widget* underMouse = this->getWidgetUnderMouse())
        if (underMouse->interactsWithStyleView())
          this->graphicsContext->drawFilledRectangle(underMouse->getAbsoluteRectangle(), Color(1, 1, 1, 0.2));
  }

  double Gui::getElapsedTime() const
  {
    return this->input->getTime();
  }

  void Gui::dispatchKeyboardEvents()
  {
    while (!this->input->isKeyboardQueueEmpty())
    {
      KeyboardInput kb = this->input->dequeueKeyboardInput();
      this->setKeyEvent(kb, false);
      switch (kb.type)
      {
        case KeyEvent::KEY_DOWN: this->handleKeyDown(kb); break;
        case KeyEvent::KEY_REPEAT: this->handleKeyRepeat(kb); break;
        case KeyEvent::KEY_UP: this->handleKeyUp(kb); break;
        default: break;
      }
    }
  }

  void Gui::dispatchMouseEvents()
  {
    while (!this->input->isMouseQueueEmpty())
    {
      MouseInput mouseInput = this->input->dequeueMouseInput();
      if (this->isUsingTransform())
      {
        float x = (float)mouseInput.x;
        float y = (float)mouseInput.y;
        this->transform.transformPoint(&x, &y);
        mouseInput.x = (int)x;
        mouseInput.y = (int)y;
      }

      if (mouseInput.type == MouseEvent::Type::MOUSE_MOVE ||
          mouseInput.type == MouseEvent::Type::MOUSE_LEAVE ||
          mouseInput.type == MouseEvent::Type::MOUSE_ENTER ||
          mouseInput.type == MouseEvent::Type::MOUSE_WHEEL_DOWN ||
          mouseInput.type == MouseEvent::Type::MOUSE_WHEEL_UP ||
          mouseInput.type == MouseEvent::Type::MOUSE_WHEEL_LEFT ||
          mouseInput.type == MouseEvent::Type::MOUSE_WHEEL_RIGHT)
        this->handleMouseAxes(mouseInput);
      else if (mouseInput.type == MouseEvent::Type::MOUSE_DOWN)
      {
        // queue it for later to fix click bug
        if (!this->queuedMouseDown.empty())
          this->queuedMouseDown.pop();
        this->queuedMouseDown.push(mouseInput);
      }
      else if (mouseInput.type == MouseEvent::Type::MOUSE_UP)
        this->handleMouseUp(mouseInput);
    }

    while (!this->queuedMouseDown.empty())
    {
      MouseInput mouseInput = this->queuedMouseDown.back();
      this->queuedMouseDown.pop();
      if (mouseInput.type == MouseEvent::Type::MOUSE_DOWN)
        this->handleMouseDown(mouseInput);
    }
  }

  void Gui::dispatchWidgetDestroyed(Widget* widget)
  {
    this->removeWidget(widget);
  }

  void Gui::add(Widget* widget)
  {
    this->baseWidget->add(widget);
    this->baseWidget->resizeNextTime(widget);
  }

  void Gui::insert(Widget *widget, uint32_t index)
  {
    this->baseWidget->insert(widget, index);
    this->baseWidget->resizeNextTime(widget);
  }

  void Gui::remove(Widget* widget)
  {
    this->baseWidget->remove(widget);
  }

  void Gui::resizeToDisplay()
  {
    if (this->graphicsContext)
      this->baseWidget->setSize(this->graphicsContext->getDisplaySize().width, this->graphicsContext->getDisplaySize().height);
    this->baseWidget->displaySizeChangedRecursive();
    for (Widget* widget : *this->baseWidget)
      this->baseWidget->resizeChild(widget);
    this->baseWidget->postDisplaySizeChangedRecursive();
  }

  void Gui::handleTimedEvents()
  {
    this->handleHover();
    this->handleDoubleClick();
  }

  void Gui::setTabNextKey(KeyEnum key,
                          ExtendedKeyEnum extKey,
                          bool shift,
                          bool control,
                          bool alt)
  {
    this->tabNextKey = key;
    this->tabNextShift = shift;
    this->tabNextControl = control;
    this->tabNextAlt = alt;
    this->tabNextExtKey = extKey;
  }

  void Gui::setTabPreviousKey(KeyEnum key,
                              ExtendedKeyEnum extKey,
                              bool shift,
                              bool control,
                              bool alt)
  {
    this->tabPreviousKey = key;
    this->tabPreviousShift = shift;
    this->tabPreviousControl = control;
    this->tabPreviousAlt = alt;
    this->tabPreviousExtKey = extKey;
  }

  bool Gui::handleTabbing()
  {
    if (this->focusManager.getFocusedWidget() && this->focusManager.getFocusedWidget()->consumesTab())
      return false;

    if ((this->tabNextKey != KEY_NONE && keyEvent.getKey() == this->tabNextKey ||
         this->tabNextExtKey != EXT_KEY_NONE &&
         this->keyEvent.getExtendedKey() == this->tabNextExtKey) &&
        this->keyEvent.shift() == this->tabNextShift &&
        this->keyEvent.control() == this->tabNextControl &&
        this->keyEvent.alt() == this->tabNextAlt)
    {
      focusNextTabableWidget();
      return true;
    }
    else if ((this->tabPreviousKey != KEY_NONE && this->keyEvent.getKey() == this->tabPreviousKey ||
              this->tabPreviousExtKey != EXT_KEY_NONE &&
              this->keyEvent.getExtendedKey() == this->tabPreviousExtKey) &&
             this->keyEvent.shift() == this->tabPreviousShift &&
             this->keyEvent.control() == this->tabPreviousControl &&
             this->keyEvent.alt() == this->tabPreviousAlt)
    {
      focusPreviousTabableWidget();
      return true;
    }

    return false;
  }

  TopContainer* Gui::getTop() const
  {
    return this->baseWidget;
  }

  bool Gui::setCursor(CursorProvider::CursorEnum cursor)
  {
    if (this->cursorProvider)
      return this->cursorProvider->setCursor(cursor);

    return false;
  }

  void Gui::forceReleaseControlWithLock()
  {
    this->controlWithLock = nullptr;
    this->commitToMouseDrag = false;
    this->mousePositionAtStartOfDrag = Point::emptyPoint();
  }

  void Gui::setCursorProvider(CursorProvider* provider)
  {
    this->cursorProvider = provider;
  }

  Widget* Gui::getLockWidget()
  {
    return *this->controlWithLock;
  }

  void Gui::requestBringWidgetToFront(Widget* widget)
  {
    this->widgetToBringToFront = widget;
  }

  void Gui::setExistanceCheck(bool check)
  {
    this->enableExistanceCheck = check;
  }

  int Gui::isDoingExistanceCheck() const
  {
    return this->enableExistanceCheck;
  }

  void Gui::modalChanged()
  {
    this->widgetLocationChanged();
  }

  ImeCompositionInfo Gui::getImeCompositionInfo()
  {
    return this->input->getImeCompositionInfo();
  }

  void Gui::setTransform(const Transform& transform)
  {
    this->transform = transform;
    this->transform.invert();
  }

  const Transform& Gui::getTransform() const
  {
    return this->transform;
  }

  void Gui::setUseTransform(bool use)
  {
    this->useTransform = use;
  }

  bool Gui::isUsingTransform() const
  {
    return this->useTransform;
  }

  void Gui::setScaleAndDisplayDensity(double scale, double displayDensity)
  {
    // Should be set together and at the same time, so let's use this function to remind setters what they need to do.
    Gui::scale = scale;
    Gui::displayDensity = displayDensity;
  }

  void Gui::clearWidgetUnderMouse()
  {
    this->widgetUnderMouse = this->getTop();
    this->updateWidgetUnderMouse = true;
  }

  void Gui::clearFocus()
  {
    this->focusManager.clearFocusedWidget();
  }

  void Gui::orderToUpdateWidgetUnderMouse()
  {
    // I need to make sure, that widgets are on their final place when selecting things under mouse
    this->getTop()->processTriggersToResize();
    this->updateWidgetUnderMouse = false;
    this->previousWidgetUnderMouse = this->widgetUnderMouse;
    this->widgetUnderMouse = this->mouseInWindow ? this->recursiveGetWidgetUnderMouse(this->currentMousePosition) : nullptr;
    if (this->focusManager.getModalWidget() &&
        !this->widgetIsModalChild(*this->widgetUnderMouse))
      this->widgetUnderMouse.clear();

    /* the locked control is valid when the mouse is down during a mouse move in order to prevent
       other widgets from receiving events until mouse up */
    if (!this->controlWithLock)
    {
      if (this->previousWidgetUnderMouse != this->widgetUnderMouse)
      {
        // invalidate the last hover control
        this->lastHoveredControl = nullptr;

        // prevent non modal event
        if (!*this->controlWithLock && !this->widgetUnderMouse)
        {
          // send previous a mouse leave
          if (this->previousWidgetUnderMouse)
            this->previousWidgetUnderMouse->dispatchMouseLeave(this->convertMouseEventToRelative(*this->previousWidgetUnderMouse,
                                                                                                 MouseEvent(this->currentMousePosition)));
          this->setCursor(this->getTop()->getEnterCursor());
          return;
        }

        // send previous a mouse leave
        if (this->previousWidgetUnderMouse)
          this->previousWidgetUnderMouse->dispatchMouseLeave(this->convertMouseEventToRelative(*this->previousWidgetUnderMouse,
                                                                                               MouseEvent(this->currentMousePosition)));

        // send the new one an enter
        if (this->widgetUnderMouse)
        {
          this->setCursor(this->widgetUnderMouse->getEnterCursor());
          MouseEvent mouseEvent(this->currentMousePosition);
          mouseEvent.previous = *this->previousWidgetUnderMouse;
          this->widgetUnderMouse->dispatchMouseEnter(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
        }
      }
    }
    else // send a mouse dragged to the locked control
    {
      if (this->previousWidgetUnderMouse != this->widgetUnderMouse)
      {
        // invalidate the last hover control
        this->lastHoveredControl = nullptr;

        // send the new one an enter
        if (this->widgetUnderMouse == this->controlWithLock && this->widgetUnderMouse)
        {
          this->setCursor(this->widgetUnderMouse->getEnterCursor());
          MouseEvent mouseEvent(this->currentMousePosition);
          mouseEvent.previous = *this->previousWidgetUnderMouse;
          this->widgetUnderMouse->dispatchMouseEnter(this->convertMouseEventToRelative(*this->widgetUnderMouse, mouseEvent));
        }

        // send previous a mouse leave
        if (this->previousWidgetUnderMouse)
          if (this->previousWidgetUnderMouse == this->controlWithLock)
            this->previousWidgetUnderMouse->dispatchMouseLeave(this->convertMouseEventToRelative(*this->previousWidgetUnderMouse, MouseEvent(currentMousePosition)));
      }
    }
  }

  void Gui::updateFlyingTexts()
  {
    this->flyingTexts.erase(std::remove_if(this->flyingTexts.begin(),
                                           this->flyingTexts.end(),
                                           [](GuiFlyingText& flyingText) { return flyingText.update(); }),
                            this->flyingTexts.end());
  }

  void Gui::addFlyingText(Point position, const std::string& text, Color color, const Font* font, int timeToLive, int screnWidth)
  {
    this->flyingTexts.emplace_back(position, text, color, font, timeToLive, screnWidth);
  }

  void Gui::callAfterResizeIsSattledAction(Widget* widget)
  {
    this->callAfterResizeIsSettledActions.emplace_back(widget);
  }

  void Gui::addAnchoredWidget(Window& window)
  {
    for (GenericTargeter<Window>& existing : this->anchoredWidgets)
      if (*existing == &window)
        return;
    this->anchoredWidgets.emplace_back(&window);
  }

  void Gui::updateTextInputInfo()
  {
    if (this->isSimulation)
      return;
    if (!this->input)
      return;
    Widget* focus = this->getFocusedWidget();
    if (!focus || focus->isFlaggedForDestruction())
    {
      this->input->updateTextInput(TextInputInfo());
      return;
    }

    TextInputInfo info = focus->queryTextInputInfo();
    if (info.enabled)
      info.pos = info.pos + focus->getAbsolutePosition();

    this->textInputInfo = info;

    this->input->updateTextInput(info);
  }

  void Gui::updateImeComposition()
  {
    const ImeCompositionInfo& info = this->getImeCompositionInfo();
    if (info.text.empty())
    {
      if (this->imeCompositionTextField)
        this->imeCompositionTextField->removeFromParent();
      return;
    }

    if (!this->imeCompositionTextField)
    {
      agui::TextField* textField = new agui::TextField(this->imeCompositionTextFieldStyle);
      textField->autoDestructWhenRemovedFromParent();
      textField->setIgnoredByInteraction(true);
      textField->setTabable(false);
      this->add(textField);
      this->imeCompositionTextField = textField;
    }

    this->imeCompositionTextField->setLocation(Point(this->textInputInfo.pos.x + this->textInputInfo.cursorOffset,
                                                     this->textInputInfo.pos.y));
    this->imeCompositionTextField->setText(info.text);
    this->imeCompositionTextField->setCaretCharPosition(agui::Point(info.cursorOffset, 0));
    this->imeCompositionTextField->setSizeToFit(info.text);
  }

  void Gui::resetState()
  {
    if (this->input)
      this->input->resetState();
    this->instantTooltip = false;
    this->timeUntilNextGuiHover = 0;
    this->timeUntilNextEntityHover = 0;
    this->wantsForceHover = false;
  }

  void Gui::anchorAnchoredWidgets()
  {
    for (size_t i = 0; i < this->anchoredWidgets.size();)
      if (Window* window = *this->anchoredWidgets[i]; window && window->checkAnchor())
        ++i;
      else
      {
        if (i + 1 != this->anchoredWidgets.size())
          this->anchoredWidgets[i] = this->anchoredWidgets.back();
        this->anchoredWidgets.pop_back();
      }
  }
}

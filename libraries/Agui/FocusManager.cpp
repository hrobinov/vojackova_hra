#include <cstdlib>
#include "Agui/FocusManager.hpp"
#include "Agui/Gui.hpp"
#include "Agui/TopContainer.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Gui.hpp"
#include "Agui/TopContainer.hpp"

namespace agui
{
  FocusManager::~FocusManager()
  {}

  void FocusManager::setFocusedWidget(Widget* widget, TabbedIn tabbedIn)
  {
    if (this->focusedWidget == widget)
      return;

    if (!widget)
    {
      this->clearFocusedWidget();
      return;
    }

    while (!widget->isFocusable() && widget->getFocusParentWhenNotFocusable() && widget->isVisible() && widget->getParent())
      widget = widget->getParent();

    if (widget->isFocusable() && widget->isVisible() && widget->isEnabled())
    {
      // if there's a modal widget, I must be its descendant to get focus
      if (auto modal = this->getModalWidget())
      {
        auto w = widget;
        while (w != modal)
        {
          w = w->getParent();
          if (!w)
            return;
        }
      }

      // I can get focus now
      this->clearFocusedWidget();
      this->focusedWidget = widget;
      widget->focusGained(tabbedIn);
    }
    else if (this->focusedWidget && !this->focusedWidget->shoulKeepFocusWhenClickingOutsideOnNonFocusableWidget())
      this->clearFocusedWidget();
  }

  void FocusManager::clearFocusedWidget()
  {
    if (this->focusedWidget)
      this->focusedWidget->focusLost();
    this->focusedWidget.clear();
  }

  Widget* FocusManager::getFocusedWidget()
  {
    return *this->focusedWidget;
  }

  void FocusManager::removeEmptyModalsOnTop()
  {
    while (!this->modals.empty() && !this->modals.back().widget)
      this->modals.pop_back();
  }

  void FocusManager::removeEmptyMainWindowsOnTop()
  {
    while (!this->mainWindows.empty() && !this->mainWindows.back())
      this->mainWindows.pop_back();
  }

  void FocusManager::checkThatModalFocusedWigetIsOnTop()
  {
    Widget* widget = this->getModalWidget();

    // No widget is focused
    if (!widget)
      return;

    // The widget has been removed from the gui, lets ignore it
    if (!widget->getParent() || !widget->getParent()->getGui())
      return;

    // Someone is trying to modal focus gui that is not on top, which doesn't make sense
    // lets abort to find out about it.
    if (widget->getParent()->getGui()->getTop() != widget->getParent())
      std::abort();

    // The widget is already on top, we don't need to do anything
    if (widget->isOnTop())
      return;

    widget->bringToFront();
  }

  void FocusManager::requestModalFocus(Widget* widget, ModalFocusPriority priority, bool isDropDownListBox)
  {
    if (!widget)
      return;

    WidgetWithPriority me = { widget, priority, isDropDownListBox };

    this->removeEmptyModalsOnTop();

    // erase me from the modal list (if I'm there)
    for (auto it = this->modals.begin(); it != this->modals.end(); ++it)
      if (it->widget == widget)
      {
        this->modals.erase(it);
        break;
      }

    // insert me into the modal list: I should be the last among widgets with the same priority
    for (auto it = this->modals.begin(); it != this->modals.end(); ++it)
      if (it->priority > priority)
      {
        this->modals.insert(it, me);  // I'm not the modal widget, no focus for me
        if (auto newModal = this->getModalWidget())
          newModal->focus();
        for (auto& widgetWithPriority : this->modals)  // reorder depth of modals
          widgetWithPriority.widget->bringToFront();
        return;
      }

    // I'll be modal, so I get focus
    this->modals.push_back(me);
    if (this->modals.back().priority == priority)
    {
      widget->focus();
      widget->bringToFront();
    }
  }

  void FocusManager::releaseModalFocus(Widget* widget)
  {
    if (!widget)
      return;

    auto modal = this->getModalWidget();
    if (modal != nullptr && modal == widget)  // I was modal
    {
      this->modals.pop_back();

      if (auto newModal = this->getModalWidget())
        newModal->focus();
    }
    else  // I wasn't modal, no focus change
    {
      for (auto it = this->modals.begin(); it != this->modals.end();)
      {
        if (!it->widget)
        {
          it = this->modals.erase(it);
          continue;
        }
        if (it->widget == widget)
        {
          this->modals.erase(it);
          break;
        }
        ++it;
      }
    }
  }

  bool FocusManager::requestTopModalFocusOrNone(Widget* widget)
  {
    if (!widget)
      return false;

    this->removeEmptyModalsOnTop();
    if (this->modals.empty())
      return false;
    this->requestModalFocus(widget, this->modals.back().priority);
    return true;
  }

  const agui::Widget* FocusManager::getModalWidget() const
  {
    for (std::vector<WidgetWithPriority>::const_reverse_iterator i = this->modals.rbegin(); i != this->modals.rend(); ++i)
      if (i->widget)
        return *i->widget;
    return nullptr;
  }

  agui::Widget* FocusManager::getModalWidget()
  {
    this->removeEmptyModalsOnTop();
    return this->modals.empty() ? nullptr : *this->modals.back().widget;
  }

  Widget* FocusManager::getModalDropDown()
  {
    this->removeEmptyModalsOnTop();
    if (!this->modals.empty() && this->modals.back().isDropDownListBox)
      return *this->modals.back().widget;
    return nullptr;
  }

  void FocusManager::onMainWindowCreated(Window* window)
  {
    this->mainWindows.emplace_back(window);
  }

  void FocusManager::onMainWindowRemoved(Window* window)
  {
    this->mainWindows.erase(std::remove(this->mainWindows.begin(), this->mainWindows.end(), window), this->mainWindows.end());
  }

  agui::Window* FocusManager::getMainWindow()
  {
    this->removeEmptyMainWindowsOnTop();
    return this->mainWindows.empty() ? nullptr : *this->mainWindows.back();
  }

}

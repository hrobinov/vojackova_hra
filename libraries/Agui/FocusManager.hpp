#pragma once
#include <Agui/Widget.hpp>
#include <Agui/GenericTargeter.hpp>
#include <Agui/ModalFocusPriority.hpp>
#include <vector>

class Window;

namespace agui
{
  class Widget;
  /** Class used to manage focus in a Gui.
   * Keeps track of focus and modal focus widgets. */
  class FocusManager
  {
  public:
    FocusManager() = default;
    virtual ~FocusManager();

    void setFocusedWidget(Widget* widget, TabbedIn tabbedIn = TabbedIn::False); // Doesn't work if it's not a child of the modal widget when there is a modal widget
    void clearFocusedWidget();
    Widget* getFocusedWidget();
    void checkThatModalFocusedWigetIsOnTop();
    bool somethingHasModalFocus() const { return !this->modals.empty(); }

  private:
    struct WidgetWithPriority
    {
      GenericTargeter<Widget> widget;
      ModalFocusPriority priority;
      //Not the same as ModalFocusPriority::DropDown, since ModalFocusPriority::DropDown has the same int value as other enum items.
      bool isDropDownListBox = false;
    };
    void removeEmptyModalsOnTop();
    void removeEmptyMainWindowsOnTop();
  public:
    /** Put into modal stack and set z-order based on it. Become the modal widget when priority is high enough. */
    void requestModalFocus(Widget* widget, ModalFocusPriority priority, bool isDropDownListBox = false);
    /** Remove from the modal stack. Can change focus to the next modal widget. */
    void releaseModalFocus(Widget* widget);
    bool requestTopModalFocusOrNone(Widget* widget);
    /** @return The modal widget or nullptr if no widget is modal. */
    Widget* getModalWidget();
    const Widget* getModalWidget() const;
    //get the top modal widget, if it's the modal ListBox of a DropDown
    Widget* getModalDropDown();

    void onMainWindowCreated(Window* window);
    void onMainWindowRemoved(Window* window);
    Window* getMainWindow();

   private:
    std::vector<WidgetWithPriority> modals;  // higher index = higher priority (or more recent if equal priority)
    std::vector<GenericTargeter<Window>> mainWindows;
    GenericTargeter<Widget> focusedWidget;
  };
}

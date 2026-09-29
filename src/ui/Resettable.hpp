// What Factorio's settings windows share: the red reset button in the
// subheader, and the way it -- and Back -- show what they would change.
//
// Every setting a page shows is tracked with the widget that shows it and a
// test of whether it differs from some other settings. Hovering the reset
// button lights up (as if hovered) every widget whose setting isn't the
// default, and hovering Back every one changed since the page opened. The
// reset button is only there to press when there is something to reset, and
// its tooltip says how much.

#pragma once

#include <app/Settings.hpp>

#include <Agui/GenericTargetable.hpp>

#include <functional>
#include <vector>

namespace agui {
class Button;
class ImageWidget;
class MouseEvent;
class Widget;
}  // namespace agui

namespace ui {

class Theme;

class Resettable : public agui::GenericTargetable {
public:
  // `onReset` puts the settings back to their defaults.
  Resettable(Theme& theme, std::function<void()> onReset);

  // The reset button, for the page's subheader.
  agui::Button& resetButton() { return *this->reset; }

  void track(agui::Widget& widget, std::function<bool(const Settings&)> differs);

  // While the mouse is on `button`, what differs from `reference()` is lit up.
  void lightWhileHovered(agui::Button& button, std::function<const Settings&()> reference);

  // After any change to the settings: the reset button's state and tooltip.
  void changed();

private:
  // Only `source` leaving puts them back, so the mouse going from one button
  // straight to the other keeps the second's.
  void highlight(const agui::Widget* source, const Settings& reference, const agui::MouseEvent& event);
  void unhighlight(const agui::Widget* source, const agui::MouseEvent& event);

  struct Setting {
    agui::Widget*                        widget;
    std::function<bool(const Settings&)> differs;
  };

  Theme&             theme;
  agui::Button*      reset = nullptr;
  agui::ImageWidget* icon  = nullptr;

  std::vector<Setting>       tracked;
  std::vector<agui::Widget*> lit;  // what highlight() lit up
  const agui::Widget*        litBy = nullptr;
};

}  // namespace ui

// The Settings page: the graphics, the interface scale among them. Taken over
// from kovarex_go_editor, less the Go board's settings.
//
// Built the way Factorio's OtherSettingsGui is: bordered frames under a
// subheader, each a setting's name with its control pushed to the right, or a
// caption over a slider; a red reset button in the subheader. Hovering the
// reset button lights up (as if hovered) every control it would put back to
// its default, and hovering Back every one changed since the page opened.
// Pressing reset only changes the page, and like any other change here that
// is only kept by Confirm.
//
// It edits a draft of the settings, taken when the page opens, and nothing
// it changes is applied until Confirm: then the App takes the draft's
// graphics settings as its own, applies them and saves them. Back (or Esc)
// just drops the draft.

#pragma once

#include <app/Settings.hpp>
#include <ui/Resettable.hpp>

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/RadioButtonGroup.hpp>
#include <Agui/Widget/Window.hpp>

#include <functional>
#include <vector>

namespace agui {
class CheckBox;
class DropDown;
class Frame;
class ImageWidget;
class LabelStyle;
class RadioButton;
class Slider;
class TextField;
class VerticalFlow;
class Widget;
}  // namespace agui

namespace ui {

class SearchBar;
class Theme;

class SettingsPage : public agui::GenericTargetable {
public:
  SettingsPage(Theme& theme, const Settings& live, std::function<void()> onConfirm, std::function<void()> onBack);

  agui::Window& root() { return this->window; }

  // The page is being opened: the draft starts as the settings are now.
  void open();

  // What the page has made of the settings, for Confirm to keep -- and for
  // the interface scale's keyboard shortcut to change while the page is up.
  Settings& draft() { return this->settings; }

  // Brings every control in line with the settings: when the page opens,
  // when changes are thrown away, and when the interface scale's keyboard
  // shortcut changes it from outside.
  void refresh();

  // What the automatic interface scale works out to for the window as it is,
  // for the label on its choice. Cheap when it hasn't changed.
  void setAutomaticScale(int percent);

  // The search in its title bar.
  SearchBar& searchBar() { return *this->search; }

private:
  // A bordered frame at the end of `content`.
  agui::Frame& section(agui::VerticalFlow& content);
  // A setting's name, followed by an info icon when it has a tooltip.
  agui::Widget& name(const char* text, const char* tip, const agui::LabelStyle* style = nullptr);
  agui::ImageWidget& info(const char* tip);
  // A row of `section`: the setting's name, and its control at the right end.
  void settingRow(agui::Frame& section, const char* name, const char* tip, agui::Widget& control);
  // A check box goes straight into its section, being its own name, followed
  // by an info icon when it has a tooltip.
  void checkRow(agui::Frame& section, agui::CheckBox& box, const char* tip);

  // After any change: the reset button is only there to press when there is
  // something to reset, and says how much.
  void changed();

  // The value typed into the manual scale's field, if it can be read.
  void typedManualScale();
  // The value typed into the tooltip delay's field, if it can be read.
  void typedTooltipDelay();

  const Settings&  live;        // the App's, as they are applied
  Settings         settings;    // the draft the page edits
  Settings         openedWith;  // the draft as the page opened, for Back's highlight
  Theme&           theme;
  agui::Window     window;
  Resettable       resettable;
  SearchBar*       search = nullptr;

  std::vector<int> fpsLimits;  // what each of the drop-down's items means
  agui::DropDown*  mode              = nullptr;
  agui::CheckBox*  vsync             = nullptr;
  agui::DropDown*  fps               = nullptr;
  agui::RadioButton* automaticScale    = nullptr;
  agui::RadioButton* manualScale       = nullptr;
  agui::RadioButtonGroup scaleChoice;
  agui::Slider*    manualScaleSlider = nullptr;
  agui::TextField* manualScaleValue  = nullptr;
  int              automatic         = 0;  // the automatic scale the label shows
  agui::Slider*    tooltipDelay      = nullptr;
  agui::TextField* tooltipDelayValue = nullptr;
};

}  // namespace ui

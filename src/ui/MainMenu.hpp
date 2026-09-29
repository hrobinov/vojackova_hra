// The main menu, as Factorio's: a column of big buttons in a window in the
// middle of the screen, and the pages they open in its place.
//
// The menu says what the player chose through takeAction(); doing it is the
// App's business.

#pragma once

#include <ui/AboutPage.hpp>
#include <ui/SettingsPage.hpp>

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

struct Settings;

namespace agui {
class Gui;
}

namespace ui {

class Theme;

class MainMenu : public agui::GenericTargetable {
public:
  enum class Action {
    None,
    NewGame,
    LoadGame,
    SaveSettings,  // Settings' Confirm: keep settings.draft()
    Quit,
  };
  enum class Page { Menu, Settings, About };

  // Adds itself to `gui`. The Settings page starts its draft from `live`.
  MainMenu(agui::Gui& gui, Theme& theme, const ::Settings& live);
  ~MainMenu();
  MainMenu(const MainMenu&) = delete;
  MainMenu& operator=(const MainMenu&) = delete;

  Page current() const { return this->page; }

  // Esc: closes the page's search if that is open, and otherwise is the
  // page's Back button. Nothing on the menu itself.
  void cancel();
  // Ctrl+F: the page's search, if it has one.
  void focusSearch();

  // Keeps what is shown in the middle of the screen. Call after Gui::logic()
  // so sizes are current.
  void layout(int screenWidth, int screenHeight);

  Action takeAction();

  SettingsPage settings;

private:
  void open(Page page);
  agui::Window& shown();

  agui::Gui&   gui;
  agui::Window window;
  AboutPage    about;

  Page   page    = Page::Menu;
  Action pending = Action::None;
  bool   recentre = true;
  int    lastScreenWidth = 0, lastScreenHeight = 0;
};

}  // namespace ui

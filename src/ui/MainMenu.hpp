// The main menu, as Factorio's: a column of big buttons in a window in the
// middle of the screen, and the pages they open in its place.
//
// The menu says what the player chose through takeAction(); doing it is the
// App's business.

#pragma once

#include <ui/AboutPage.hpp>

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

namespace agui {
class Gui;
}

namespace ui {

class Theme;

class MainMenu : public agui::GenericTargetable {
public:
  enum class Action { None, NewGame, LoadGame, Quit };

  // Adds itself to `gui`.
  MainMenu(agui::Gui& gui, Theme& theme);
  ~MainMenu();
  MainMenu(const MainMenu&) = delete;
  MainMenu& operator=(const MainMenu&) = delete;

  // Back from a page to the menu; nothing on the menu itself.
  void back();

  // Keeps what is shown in the middle of the screen. Call after Gui::logic()
  // so sizes are current.
  void layout(int screenWidth, int screenHeight);

  Action takeAction();

private:
  enum class Page { Menu, About };

  void open(Page page);
  agui::Window& current();

  agui::Gui&   gui;
  agui::Window window;
  AboutPage    about;

  Page   page    = Page::Menu;
  Action pending = Action::None;
  bool   recentre = true;
  int    lastScreenWidth = 0, lastScreenHeight = 0;
};

}  // namespace ui

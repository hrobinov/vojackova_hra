// The Agui side of the screen: the raylib backend, the theme, the Gui itself,
// and the main menu living in it. Owns them in the order they have to be built
// and torn down: widgets reference the theme's fonts and textures, so the
// theme outlives them.

#pragma once

#include <ui/AguiRaylib.hpp>

#include <memory>

struct Settings;

namespace agui {
class Gui;
}

namespace ui {

class MainMenu;
class Theme;

class GuiLayer {
public:
  // Needs the window. The Settings page starts its draft from `settings`.
  explicit GuiLayer(const Settings& settings);
  ~GuiLayer();
  GuiLayer(const GuiLayer&) = delete;
  GuiLayer& operator=(const GuiLayer&) = delete;

  // Once per frame, before anything asks the menu what happened.
  void update();

  // The interface scale, in percent: everything is laid out on a display that
  // much smaller than the window and drawn that much bigger. Takes effect as a
  // resize on the next update().
  void setScale(int percent);

  // How long the mouse rests on something before its tooltip shows, in
  // milliseconds; negative for never. Shift shows them at once regardless.
  void setTooltipDelay(int milliseconds);

  void draw();

  MainMenu& menu() { return *this->mainMenu; }

private:
  agui_raylib::RaylibFontLoader     fontLoader;
  agui_raylib::RaylibGraphics       graphics;
  agui_raylib::RaylibInput          input;
  agui_raylib::RaylibCursorProvider cursor;

  std::unique_ptr<Theme>     theme;
  std::unique_ptr<agui::Gui> gui;
  std::unique_ptr<MainMenu>  mainMenu;

  int screenWidth  = 0;  // what the Gui was last sized to, in GUI units
  int screenHeight = 0;
};

}  // namespace ui

// A small window of someone's basic properties -- a row each for actions,
// lives, wounds and, for the figure, its pistol's range, and its rockets and
// shotgun shells once it has bought any; the value at the right end -- as
// the game's menu shows them for the figure and for the white zombie.

#pragma once

#include <game/Game.hpp>

#include <Agui/Widget/Window.hpp>

#include <string>

namespace agui {
class Label;
class Widget;
}  // namespace agui

namespace ui {

class Theme;

class PropertiesWindow : public agui::Window {
public:
  // `armed`: the figure's, with the pistol's range, rockets and shells.
  PropertiesWindow(Theme& theme, const std::string& title, bool armed);

  void show(const Properties& properties);

  // Wide enough for the longest title.
  static constexpr int PANEL_W = 240;

private:
  agui::Label* actions = nullptr;
  agui::Label* lives   = nullptr;
  agui::Label* wounds  = nullptr;
  agui::Label* range   = nullptr;
  agui::Label* rockets = nullptr;
  agui::Label* shells  = nullptr;
  // The rows of the rockets and the shells, there only once bought.
  agui::Widget* rocketsRow = nullptr;
  agui::Widget* shellsRow  = nullptr;
  bool          armed      = false;
};

}  // namespace ui

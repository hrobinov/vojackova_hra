// A small window of someone's basic properties -- a row each for actions,
// lives, wounds and, for someone with a pistol, range, the value at the
// right end -- as the game's menu shows them for the figure and for the
// white zombie.

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
  PropertiesWindow(Theme& theme, const std::string& title);

  void show(const Properties& properties);

  // Wide enough for the longest title.
  static constexpr int PANEL_W = 240;

private:
  agui::Label* actions = nullptr;
  agui::Label* lives   = nullptr;
  agui::Label* wounds  = nullptr;
  agui::Label* range   = nullptr;
  agui::Widget* rangeRow = nullptr;
};

}  // namespace ui

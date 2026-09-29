// A small window of someone's basic properties -- a row each for actions,
// lives and wounds, the value at the right end -- as the game's menu shows
// them for the figure and for the white zombie.

#pragma once

#include <game/Game.hpp>

#include <Agui/Widget/Window.hpp>

#include <string>

namespace agui {
class Label;
}

namespace ui {

class Theme;

class PropertiesWindow : public agui::Window {
public:
  PropertiesWindow(Theme& theme, const std::string& title);

  void show(const Properties& properties);

private:
  agui::Label* actions = nullptr;
  agui::Label* lives   = nullptr;
  agui::Label* wounds  = nullptr;
};

}  // namespace ui

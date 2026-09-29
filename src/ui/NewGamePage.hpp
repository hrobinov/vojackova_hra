// The New game page: what the new game is to be called, at most
// Game::MAX_NAME characters: a character past that can't be typed or pasted
// in, and a red line under the name says why. Done (or Enter) starts it; Done is only there
// to press once there is a name.

#pragma once

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

#include <functional>
#include <string>

namespace agui {
class Button;
class Label;
class TextField;
}  // namespace agui

namespace ui {

class Theme;

class NewGamePage : public agui::GenericTargetable {
public:
  NewGamePage(Theme& theme, std::function<void()> onDone, std::function<void()> onBack);

  agui::Window& root() { return this->window; }

  // The page is being opened: the name starts empty, ready to type.
  void open();

  // The name typed, without the spaces round it.
  std::string name() const;

private:
  // After every edit: Done is only there to press with a name.
  void edited();
  void done();

  agui::Window     window;
  agui::TextField* field   = nullptr;
  agui::Label*     counter = nullptr;  // "3 / 15"
  agui::Label*     tooLong = nullptr;  // red, after a character too many
  agui::Button*    finish  = nullptr;
  std::function<void()> onDone;
};

}  // namespace ui

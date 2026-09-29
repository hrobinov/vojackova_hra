// "Opravdu chceš smazat hru Hra 3?" -- what the Load game page's trash can
// asks before a save is gone for good. Cancel (or Esc) leaves it be.

#pragma once

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

#include <functional>
#include <string>

namespace agui {
class Label;
}

namespace ui {

class Theme;

class DeleteGamePage : public agui::GenericTargetable {
public:
  DeleteGamePage(Theme& theme, std::function<void()> onDelete, std::function<void()> onCancel);

  agui::Window& root() { return this->window; }

  // The game the page asks about.
  void setGame(const std::string& name);

private:
  agui::Window window;
  agui::Label* question = nullptr;
};

}  // namespace ui

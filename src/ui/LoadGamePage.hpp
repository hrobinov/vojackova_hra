// The Load game page: every saved game, the last saved first, each with when
// it was saved and a red trash can button. Clicking a game loads it; the
// trash can asks, on the Delete game page, whether to delete it.

#pragma once

#include <game/Game.hpp>

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

#include <filesystem>
#include <functional>
#include <vector>

namespace agui {
class Label;
class VerticalFlow;
}  // namespace agui

namespace ui {

class Theme;

class LoadGamePage : public agui::GenericTargetable {
public:
  LoadGamePage(Theme& theme, std::function<void(const std::filesystem::path&)> onChosen,
               std::function<void(const SavedGame&)> onDelete, std::function<void()> onBack);

  agui::Window& root() { return this->window; }

  // The page is being opened, or a save was deleted: lists the saves as
  // they are now.
  void open();

private:
  Theme&              theme;
  agui::Window        window;
  agui::VerticalFlow* rows  = nullptr;  // a row a save
  agui::Label*        empty = nullptr;  // in place of the rows while there are no saves

  std::vector<SavedGame> saves;  // what each row is
  std::function<void(const std::filesystem::path&)> onChosen;
  std::function<void(const SavedGame&)>             onDelete;
};

}  // namespace ui

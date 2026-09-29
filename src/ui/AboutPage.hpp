// The About page: who is making the game.

#pragma once

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/Window.hpp>

#include <functional>

namespace ui {

class Theme;

class AboutPage : public agui::GenericTargetable {
public:
  AboutPage(Theme& theme, std::function<void()> onBack);

  agui::Window& root() { return this->window; }

private:
  agui::Window window;
};

}  // namespace ui

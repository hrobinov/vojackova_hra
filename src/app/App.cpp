#include <app/App.hpp>

#include <app/Config.hpp>
#include <ui/Fonts.hpp>
#include <ui/MainMenu.hpp>

#include <raylib.h>

App::Window::Window()
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(cfg::WINDOW_W, cfg::WINDOW_H, cfg::APP_NAME);
  SetWindowMinSize(cfg::MIN_WINDOW_W, cfg::MIN_WINDOW_H);
  // Esc belongs to the menu; closing is the close button's job.
  SetExitKey(KEY_NULL);
}

App::Window::~Window()
{
  ui::UnloadFonts();
  CloseWindow();
}

int App::run()
{
  while (!this->quitRequested) this->frame();
  return 0;
}

void App::frame()
{
  this->gui.update();

  // Esc is the About page's Back button.
  if (IsKeyPressed(KEY_ESCAPE)) this->gui.menu().back();
  this->handleMenu();
  if (WindowShouldClose()) this->quitRequested = true;

  BeginDrawing();
  ClearBackground(cfg::BACKGROUND_COLOR);
  this->gui.draw();
  EndDrawing();
}

void App::handleMenu()
{
  switch (this->gui.menu().takeAction()) {
  case ui::MainMenu::Action::NewGame:
    // Nothing yet: there is no game to start.
    break;
  case ui::MainMenu::Action::LoadGame:
    // Nothing yet: there is nothing to load.
    break;
  case ui::MainMenu::Action::Quit:
    this->quitRequested = true;
    break;
  case ui::MainMenu::Action::None:
    break;
  }
}

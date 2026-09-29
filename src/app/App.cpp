#include <app/App.hpp>

#include <app/Config.hpp>
#include <ui/Fonts.hpp>
#include <ui/MainMenu.hpp>

#include <raylib.h>

App::Window::Window(const Settings& settings)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | (settings.graphics.vsync ? FLAG_VSYNC_HINT : 0));
  InitWindow(settings.window.width, settings.window.height, cfg::APP_NAME);
  SetWindowMinSize(cfg::MIN_WINDOW_W, cfg::MIN_WINDOW_H);
  // Esc belongs to the menu; closing is the close button's job.
  SetExitKey(KEY_NULL);
  if (settings.window.maximized) MaximizeWindow();
}

App::Window::~Window()
{
  ui::UnloadFonts();
  CloseWindow();
}

App::App()
{
  ApplySettings(this->settings);
}

int App::run()
{
  while (!this->quitRequested) this->frame();
  this->settings.save();  // for the window size, if nothing else
  return 0;
}

void App::frame()
{
  this->handleKeys();
  // Settings next: a change made last frame -- a new interface scale, say --
  // is in place before the Gui lays itself out for this one.
  this->updateSettings();
  this->gui.update();
  this->handleMenu();
  if (WindowShouldClose()) this->quitRequested = true;

  BeginDrawing();
  ClearBackground(cfg::BACKGROUND_COLOR);
  this->gui.draw();
  EndDrawing();
}

void App::handleKeys()
{
  ui::MainMenu& menu = this->gui.menu();
  if (IsKeyPressed(KEY_ESCAPE)) menu.cancel();

  const bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  if (!ctrl) return;
  if (IsKeyPressed(KEY_F)) menu.focusSearch();
  if (IsKeyPressed(KEY_KP_ADD)) this->scale(Scale::Up);
  if (IsKeyPressed(KEY_KP_SUBTRACT)) this->scale(Scale::Down);
  if (IsKeyPressed(KEY_KP_0)) this->scale(Scale::Automatic);
}

void App::handleMenu()
{
  ui::MainMenu& menu = this->gui.menu();
  switch (menu.takeAction()) {
  case ui::MainMenu::Action::NewGame:
    // Nothing yet: there is no game to start.
    break;
  case ui::MainMenu::Action::LoadGame:
    // Nothing yet: there is nothing to load.
    break;
  case ui::MainMenu::Action::SaveSettings:
    // Only now does anything the page changed take effect: updateSettings()
    // applies it from here on.
    this->settings.graphics = menu.settings.draft().graphics;
    this->settings.save();
    break;
  case ui::MainMenu::Action::Quit:
    this->quitRequested = true;
    break;
  case ui::MainMenu::Action::None:
    break;
  }
}

void App::scale(Scale how)
{
  // A step up or down, from anywhere -- from the automatic scale, which it
  // leaves for a custom one, as Factorio's do -- or back to automatic. Kept
  // and applied at once, unless the settings page is up: there it is one
  // more change to the page's draft, which only Confirm keeps.
  ui::MainMenu&       menu     = this->gui.menu();
  const bool          onPage   = menu.current() == ui::MainMenu::Page::Settings;
  Settings::Graphics& graphics = onPage ? menu.settings.draft().graphics : this->settings.graphics;
  if (how == Scale::Automatic) {
    if (graphics.automaticScale) return;
    graphics.automaticScale = true;
  } else {
    const int step = how == Scale::Up ? Settings::Graphics::SCALE_STEP : -Settings::Graphics::SCALE_STEP;
    const int from = graphics.automaticScale ? AutomaticInterfaceScale(GetScreenWidth(), GetScreenHeight())
                                             : graphics.interfaceScale;
    graphics.interfaceScale = ClampedInterfaceScale(from + step);
    graphics.automaticScale = false;
  }
  if (onPage) menu.settings.refresh();
  else        this->settings.save();
}

void App::updateSettings()
{
  this->gui.menu().settings.setAutomaticScale(AutomaticInterfaceScale(GetScreenWidth(), GetScreenHeight()));

  // The scale drawn at: on automatic, it follows the window as it is resized.
  const Settings::Graphics& graphics = this->settings.graphics;
  if (const int scale = EffectiveInterfaceScale(graphics, GetScreenWidth(), GetScreenHeight()); scale != this->shownScale) {
    this->gui.setScale(scale);
    this->shownScale = scale;
  }

  // Bring the window and the Gui in line with the settings, when Confirm on
  // the settings page (or the scale's shortcut) has changed them.
  if (this->settings.graphics != this->applied.graphics) {
    if (this->settings.graphics.tooltipDelay != this->applied.graphics.tooltipDelay) {
      this->gui.setTooltipDelay(this->settings.graphics.tooltipDelay);
    }
    ApplySettings(this->settings);
    this->applied = this->settings;
  }

  // Remember the window as the player left it. Not while windowed fullscreen,
  // which would record the monitor's size, nor maximized, which would record
  // the maximized size in place of the one to restore to, nor minimized, which
  // Windows reports as 0x0.
  if (!IsWindowedFullscreen() && !IsWindowMinimized()) {
    this->settings.window.maximized = IsWindowMaximized();
    if (!this->settings.window.maximized) {
      this->settings.window.width  = GetScreenWidth();
      this->settings.window.height = GetScreenHeight();
    }
  }
}

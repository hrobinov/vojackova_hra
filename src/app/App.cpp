#include <app/App.hpp>

#include <app/Config.hpp>
#include <game/BoardView.hpp>
#include <ui/Fonts.hpp>
#include <ui/MainMenu.hpp>

#include <raylib.h>
#include <rlgl.h>

#include <algorithm>

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
  if (this->game && this->gui.menu().gobanShown()) this->bankEarnings();
  if (this->game) this->game->save();
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
  this->updateZombie();
  // With no lives left the figure is dead: after a moment to see it, back to
  // the game's menu, as Back to menu would. The moment waits while paused.
  if (this->game && this->game->goban.figureLives() == 0 && this->gui.menu().gobanShown()) {
    if (this->gui.menu().current() == ui::MainMenu::Page::Goban) this->deathWait += GetFrameTime();
    if (this->deathWait >= cfg::DEATH_SECONDS) {
      this->deathWait = 0.0f;
      this->bankEarnings();
      this->gui.menu().leaveGoban();
    }
  } else {
    this->deathWait = 0.0f;
  }
  // How far the game has got, kept -- and saved -- with the earnings.
  if (this->game) this->game->furthestWave = std::max(this->game->furthestWave, this->game->goban.waveNumber());
  if (this->game) this->gui.menu().showRound(this->game->goban);
  if (WindowShouldClose()) this->quitRequested = true;

  // Explosions only while the goban is there to show them.
  if (this->gui.menu().gobanShown()) this->explosions.update(GetFrameTime());
  else                               this->explosions.clear();

  BeginDrawing();
  ClearBackground(cfg::BACKGROUND_COLOR);
  if (this->game && this->gui.menu().gobanShown()) {
    // The goban and the explosions on it, shaken as a rocket goes off.
    const BoardLayout goban = LayOutBoard(GetScreenWidth(), GetScreenHeight());
    const Vector2     shake = this->explosions.shake(goban);
    rlPushMatrix();
    rlTranslatef(shake.x, shake.y, 0.0f);
    DrawBoard(this->game->goban, GetScreenWidth(), GetScreenHeight());
    this->explosions.draw(goban);
    rlPopMatrix();
  }
  this->gui.draw();
  EndDrawing();
}

void App::handleKeys()
{
  ui::MainMenu& menu = this->gui.menu();
  if (IsKeyPressed(KEY_ESCAPE)) menu.cancel();
  // F3 opens the command line, with a game to type to.
  if (IsKeyPressed(KEY_F3) && this->game) menu.toggleCommandLine();

  // The arrow keys, or W A S D, move the figure, a square a press -- and on
  // and on while one is held, as a key held in a text field repeats. Not
  // while paused, nor while typing into the command line; while the zombies
  // move, Goban::move() does nothing.
  if (this->game && menu.current() == ui::MainMenu::Page::Goban && !menu.commandLineOpen()) {
    const auto pressed = [](int arrow, int letter) {
      return IsKeyPressed(arrow) || IsKeyPressedRepeat(arrow) || IsKeyPressed(letter) || IsKeyPressedRepeat(letter);
    };
    if (pressed(KEY_LEFT, KEY_A)) this->game->goban.move(-1, 0);
    if (pressed(KEY_RIGHT, KEY_D)) this->game->goban.move(1, 0);
    if (pressed(KEY_UP, KEY_W)) this->game->goban.move(0, -1);
    if (pressed(KEY_DOWN, KEY_S)) this->game->goban.move(0, 1);
    // Space waits. Only on a press: held, it would throw the turn away.
    if (IsKeyPressed(KEY_SPACE)) this->game->goban.wait();

    // A click on a square shoots at it -- which only does anything on a
    // zombie, in range -- or with a special ability readied, uses that: a
    // rocket anywhere, the shotgun right beside the figure. It is gone for
    // this round; the next Start has it back.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const BoardLayout goban = LayOutBoard(GetScreenWidth(), GetScreenHeight());
      const Vector2     mouse = GetMousePosition();
      const int         x     = int(mouse.x) - goban.left;
      const int         y     = int(mouse.y) - goban.top;
      if (x >= 0 && y >= 0 && x < goban.side && y < goban.side) {
        const Position at{ x / goban.square, y / goban.square };
        Goban& round = this->game->goban;
        switch (menu.readied()) {
        case ui::MainMenu::Special::None:
          round.shoot(at);
          break;
        case ui::MainMenu::Special::Rocket:
          if (round.fireRocket(at)) {
            menu.disarm();
            this->explosions.addRocket(at);
          }
          break;
        case ui::MainMenu::Special::Shotgun: {
          const Position from = round.figureAt();
          if (round.fireShotgun(at)) {
            menu.disarm();
            this->explosions.addShotgun(from, *Goban::ShotgunSpread(from, at));
          }
          break;
        }
        }
      }
    }
  }

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
    this->play(Game::New(menu.newGameName()));
    break;
  case ui::MainMenu::Action::LoadGame:
    if (std::optional<Game> loaded = Game::Load(menu.chosenSave())) this->play(std::move(*loaded));
    else TraceLog(LOG_WARNING, "GAME: Couldn't read %s", menu.chosenSave().string().c_str());
    break;
  case ui::MainMenu::Action::StartGoban:
    // From the beginning, or a checkpoint -- never where the last round was.
    if (this->game) this->game->goban.start(this->game->figure, this->game->zombie, menu.startWave());
    break;
  case ui::MainMenu::Action::LeaveGoban:
    this->bankEarnings();
    break;
  case ui::MainMenu::Action::BuyUpgrade:
    // Saved at once, as money is.
    if (this->game && this->game->buy(menu.boughtUpgrade())) {
      this->game->save();
      menu.showGame(*this->game);
    }
    break;
  case ui::MainMenu::Action::Command:
    if (this->game && this->game->enterPassword(menu.command())) {
      this->game->save();
      menu.showGame(*this->game);
    }
    break;
  case ui::MainMenu::Action::SaveAndQuit:
    if (this->game) this->game->save();
    this->game.reset();
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

void App::bankEarnings()
{
  if (!this->game) return;
  this->game->money += this->game->goban.takeEarnings();
  this->game->save();
  this->gui.menu().showGame(*this->game);
}

void App::updateZombie()
{
  // An action every ZOMBIE_STEP_SECONDS, the first one too, so the figure's last
  // action can be seen before the zombie answers it. Held while paused.
  if (!this->game || !this->game->goban.zombiesTurn()) {
    this->zombieWait = 0.0f;
    return;
  }
  if (this->gui.menu().current() != ui::MainMenu::Page::Goban) return;
  this->zombieWait += GetFrameTime();
  if (this->zombieWait < cfg::ZOMBIE_STEP_SECONDS) return;
  this->zombieWait -= cfg::ZOMBIE_STEP_SECONDS;
  this->game->goban.zombieAction();
}

void App::play(Game started)
{
  this->game = std::move(started);
  this->gui.menu().play(*this->game);
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

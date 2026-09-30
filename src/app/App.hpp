// The application: the window, the settings, the game being played and the
// frame loop. Each frame the keyboard shortcuts are read first, then the Gui
// runs, then whatever the menu asked for is done, then everything is drawn:
// the game, and the Gui over it.

#pragma once

#include <app/Settings.hpp>
#include <game/Explosions.hpp>
#include <game/Game.hpp>
#include <ui/GuiLayer.hpp>

#include <optional>

class App {
public:
  App();

  // Runs until the window closes.
  int run();

private:
  // Opens the window on construction and closes it on destruction. Declared
  // before every member that needs the GL context, so it brackets them.
  struct Window {
    explicit Window(const Settings& settings);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
  };

  // The interface scale's shortcuts: a step up or down, or back to automatic.
  enum class Scale { Up, Down, Automatic };

  void frame();
  void handleKeys();
  void handleMenu();
  // While it is the zombie's turn on the goban: its actions, one at a time.
  void updateZombie();
  // The round's earnings into the game's money, which is saved at once, so
  // it can't be lost. When the goban is left, however it is.
  void bankEarnings();
  // Starts playing `started`.
  void play(Game started);
  void updateSettings();
  void scale(Scale how);

  // Read before the window opens, which is sized from it.
  Settings settings = Settings::load();
  Settings applied  = this->settings;  // what the window was last set to
  int      shownScale = 0;             // the interface scale the GUI is drawn at

  Window       window{ this->settings };
  ui::GuiLayer gui{ this->settings };

  // The game being played, if any. Saved when the window closes mid-game,
  // as Save and quit would.
  std::optional<Game> game;
  float zombieWait = 0.0f;  // seconds since the zombie's last step
  float deathWait  = 0.0f;  // seconds the figure has been dead, on the goban
  Explosions explosions;    // the rockets going off on the goban

  bool quitRequested = false;
};

// The application: the window, the settings, the game being played and the
// frame loop. Each frame the keyboard shortcuts are read first, then the Gui
// runs, then whatever the menu asked for is done, then everything is drawn:
// the game, and the Gui over it.

#pragma once

#include <app/Settings.hpp>
#include <game/Explosions.hpp>
#include <game/Game.hpp>
#include <game/Motion.hpp>
#include <game/Scenery.hpp>
#include <game/Sounds.hpp>
#include <game/Splatters.hpp>
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

  // A zombie killed on the goban, to be shown dying -- its body falling, its
  // puddle, and but for a rocket's, its groan -- once `delay` has run out:
  // when the rocket that killed it has landed. And the bang of a rocket, as
  // it lands.
  struct Dying {
    Goban::Zombie           zombie;
    float                   delay;
    std::optional<Position> blast;  // where the rocket that killed it hit
  };

  // The interface scale's shortcuts: a step up or down, or back to automatic.
  enum class Scale { Up, Down, Automatic };

  void frame();
  void handleKeys();
  void handleMenu();
  // While it is the zombie's turn on the goban: its actions, one at a time.
  void updateZombie();
  // The zombies killed since, and the soldier if he has been: bodies,
  // puddles, groans, his fall and his cry. Killed by a rocket that hit
  // `blast`, `delay` till it lands, more blood, and without a groan: its
  // bang is enough.
  void updateDeaths(float delay = 0.0f, std::optional<Position> blast = std::nullopt);
  // The goban's square under the mouse, if it is over one.
  std::optional<Position> squareUnderMouse() const;
  // With a rocket or the shotgun readied: a faint red over the squares it
  // would hit, fired at the square under the mouse.
  void drawAim(const BoardLayout& goban);
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
  Scenery    scenery;       // the forest the goban lies in
  Motion     motion;        // everyone on it gliding and swaying
  Splatters  splatters;     // what the dead leave on it
  Sounds     sounds;
  std::vector<Dying> dying;
  std::vector<float> bangs;  // rockets yet to land, how long till they do
  bool       soldierDown = false;  // his fall begun, and his cry heard
  int        waveSeen    = 0;      // the wave last seen on the goban
  bool       waveOver    = false;  // its dead to fade, once all have fallen

  bool quitRequested = false;
};

// The main menu, as Factorio's: a column of big buttons in a window in the
// middle of the screen, and the pages they open in its place.
//
// A game, started or loaded, opens on its own menu: for now only a Start
// button, which opens the goban. Esc on the goban brings up a pause menu over
// it with Back to menu, back to the game's menu; Esc on the game's menu, one
// with Save and quit, back to the main menu. Esc again closes either.
//
// The menu says what the player chose through takeAction(); doing it is the
// App's business.

#pragma once

#include <ui/AboutPage.hpp>
#include <ui/DeleteGamePage.hpp>
#include <ui/LoadGamePage.hpp>
#include <ui/NewGamePage.hpp>
#include <ui/PropertiesWindow.hpp>
#include <ui/UpgradeWindow.hpp>
#include <ui/SettingsPage.hpp>

#include <Agui/GenericTargetable.hpp>
#include <Agui/Widget/EmptyWidget.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/Window.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct Settings;

namespace agui {
class Button;
class VerticalFlow;
class Gui;
class TextField;
}  // namespace agui

namespace ui {

class Theme;

class MainMenu : public agui::GenericTargetable {
public:
  enum class Action {
    None,
    NewGame,       // the New game page's Done: start a game called newGameName()
    LoadGame,      // a save on the Load game page: load chosenSave()
    SaveSettings,  // Settings' Confirm: keep settings.draft()
    SaveAndQuit,   // the game menu's pause menu: save the game, and back to the main menu
    StartGoban,    // the game menu's Start, or a checkpoint: a new round on the goban, at startWave()
    LeaveGoban,    // the goban's pause menu: back to the game's menu, the round's earnings banked
    BuyUpgrade,    // an upgrade in the shop: buy UPGRADES[boughtUpgrade()]
    Command,       // Enter in the line F3 opens: try command()
    Quit,
  };
  // Game is the game's menu, and Goban the goban, which has the screen to
  // itself; each has a pause menu.
  enum class Page { Menu, NewGame, LoadGame, DeleteGame, Settings, About, Game, GamePause, Goban, GobanPause };

  // Adds itself to `gui`. The Settings page starts its draft from `live`.
  MainMenu(agui::Gui& gui, Theme& theme, const ::Settings& live);
  ~MainMenu();
  MainMenu(const MainMenu&) = delete;
  MainMenu& operator=(const MainMenu&) = delete;

  Page current() const { return this->page; }

  // A game has started: the main menu makes way for its menu.
  void play(const ::Game& game);

  // Brings the game's menu in line with `game`: the properties, the money,
  // and what of the shop there is money for.
  void showGame(const ::Game& game);

  // Whether the goban is on the screen, paused or not.
  bool gobanShown() const { return this->page == Page::Goban || this->page == Page::GobanPause; }

  // Back from the goban to the game's menu, as the goban's pause menu's Back
  // to menu does: when the figure dies.
  void leaveGoban() { this->open(Page::Game); }

  // Round the goban: right of it the figure's lives, what is left of its
  // turn and the round's earnings; left of it the special abilities it has;
  // and over it the wave. Cheap when they haven't changed.
  void showRound(const Goban& goban);

  // The special abilities the buttons left of the goban ready, so the next
  // click on the goban uses one rather than the pistol; and put back after.
  enum class Special { None, Rocket, Shotgun };
  Special readied() const { return this->armed; }
  void    disarm();



  // Esc: closes the page's search if that is open, and otherwise is the
  // page's Back button. Nothing on the menu itself. While playing it brings
  // up a pause menu, and takes it away again.
  void cancel();
  // Ctrl+F: the page's search, if it has one.
  void focusSearch();

  // Keeps what is shown in the middle of the screen. Call after Gui::logic()
  // so sizes are current.
  void layout(int screenWidth, int screenHeight);

  Action takeAction();

  // What NewGame's game is to be called.
  std::string newGameName() const { return this->newGame.name(); }
  // The wave StartGoban starts at.
  int startWave() const { return this->startingWave; }
  // The upgrade BuyUpgrade is for, an index into UPGRADES.
  size_t boughtUpgrade() const { return this->bought; }
  // F3, with a game open: the line at the bottom of the screen to type into,
  // as a chat's. Enter sends what was typed, as Command, and closes it; so
  // does F3 again, or Esc, without sending anything.
  void toggleCommandLine();
  bool commandLineOpen() const;
  // What Command is for.
  std::string command() const { return this->sent; }
  // The save LoadGame is for.
  const std::filesystem::path& chosenSave() const { return this->chosen; }

  SettingsPage settings;

private:
  void open(Page page);
  // What is in the middle of the screen; nothing while playing.
  agui::Window* shown();

  agui::Gui&   gui;
  agui::Window window;
  NewGamePage    newGame;
  LoadGamePage   loadGame;
  DeleteGamePage deleteGame;
  AboutPage    about;
  // The game's menu; the sheet that darkens the screen behind a pause menu,
  // and the pause menus, of the game's menu and of the goban.
  agui::Window      game;
  // In the game menu's bottom right corner: the basic properties of the
  // figure, and over them the white zombie's, the black one's and the red one's.
  PropertiesWindow  figure;
  PropertiesWindow  zombie;
  PropertiesWindow  blackZombie;
  PropertiesWindow  redZombie;
  // Beside the goban: the figure's lives, and the round's earnings.
  agui::Frame       round;
  agui::Label*      livesText     = nullptr;
  agui::Label*      earningsText  = nullptr;
  int               livesShown    = -1;
  int               earningsShown = -1;
  agui::Label*      actionsText   = nullptr;
  int               actionsShown  = -1;
  // Left of the goban, one under another: each special ability the figure
  // has, how many it has left, and the button that readies one.
  struct SpecialRow {
    Special             special = Special::None;
    const char*         name    = nullptr;  // "Rakety"
    const char*         ready   = nullptr;  // the button: "Aktivovat raketu"
    const char*         aim     = nullptr;  // the button once pressed: "Klikni na cíl"
    agui::VerticalFlow* row     = nullptr;
    agui::Label*        text    = nullptr;
    agui::Button*       button  = nullptr;
    int                 shown   = -1;
  };
  agui::Frame       specialBox;
  SpecialRow        specials[2];
  Special           armed = Special::None;
  // Over the goban: which wave it is.
  agui::Label       waveTitle;
  int               waveShown = -1;
  // In the game's menu, over the properties: the game's money.
  agui::Window      money;
  agui::Label*      moneyText = nullptr;
  // In the game's menu: the shop, an upgrade a window -- from the top left
  // along the top, and under them down the left, headed, the special
  // abilities.
  std::vector<std::unique_ptr<UpgradeWindow>> shop;
  agui::Label       specialTitle;
  size_t            bought = 0;
  // At the bottom of the screen, while F3 has it open: the command line.
  agui::Frame       commandLine;
  agui::TextField*  commandField = nullptr;
  std::string       sent;
  // Right of Start: a button for each checkpoint reached, in rows of a few.
  agui::VerticalFlow* checkpoints     = nullptr;
  int                 checkpointsShown = 0;
  int                 startingWave     = 1;
  // A new round, at wave `wave`.
  void startAt(int wave);
  agui::EmptyWidget dimmer;
  agui::Window      gamePause;
  agui::Window      gobanPause;

  std::filesystem::path chosen;
  std::filesystem::path toDelete;  // the save the Delete game page asks about

  Page   page    = Page::Menu;
  Action pending = Action::None;
  bool   recentre = true;
  int    lastScreenWidth = 0, lastScreenHeight = 0;
};

}  // namespace ui

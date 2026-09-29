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
#include <Agui/Widget/Window.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct Settings;

namespace agui {
class Gui;
class Label;
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
    StartGoban,    // the game menu's Start: a new round on the goban
    LeaveGoban,    // the goban's pause menu: back to the game's menu, the round's earnings banked
    BuyUpgrade,    // an upgrade in the shop: buy UPGRADES[boughtUpgrade()]
    EnterPassword, // the secret password's field confirmed: try password()
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

  // The figure's lives and the round's earnings, beside the goban. Cheap
  // when they haven't changed. (The zombies' lives are the game's business,
  // and not shown.)
  void showRound(int lives, int earnings);



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
  // The upgrade BuyUpgrade is for, an index into UPGRADES.
  size_t boughtUpgrade() const { return this->bought; }
  // What EnterPassword is for; and the field emptied again after.
  std::string password() const;
  void        clearPassword();
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
  // figure, and over them the white zombie's.
  PropertiesWindow  figure;
  PropertiesWindow  zombie;
  // Beside the goban: the figure's lives, and the round's earnings.
  agui::Frame       round;
  agui::Label*      livesText     = nullptr;
  agui::Label*      earningsText  = nullptr;
  int               livesShown    = -1;
  int               earningsShown = -1;
  // In the game's menu, over the properties: the game's money.
  agui::Window      money;
  agui::Label*      moneyText = nullptr;
  // In the game's menu, from the top left along the top: the shop, an
  // upgrade a window. And over Start, the secret password.
  std::vector<std::unique_ptr<UpgradeWindow>> shop;
  size_t            bought = 0;
  agui::TextField*  passwordField = nullptr;
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

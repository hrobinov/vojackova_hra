// A game: its name, the properties of its figure and its zombies, its money
// and what it has bought with it, how far it has got, the goban being
// played, and the save it lives in between runs. What happens on the
// goban isn't saved: every Start begins it anew (see Goban).
//
// Saves are INI files, one per game, in %APPDATA%\VojackovaHra\saves\ (next
// to config.ini). A new game gets a save of its own, which Save and quit then
// writes; loading a game and saving it again writes over the same one.

#pragma once

#include <game/Goban.hpp>
#include <game/Upgrade.hpp>

#include <array>
#include <ctime>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Game {
  // The board the save keeps is SIZE x SIZE squares, as the goban is.
  static constexpr int SIZE = Goban::SIZE;

  // What is on a square, as its save writes it. So far there is nothing to
  // put anywhere, so every square is empty.
  static constexpr char EMPTY = '.';

  // The longest name a game can have, in characters.
  static constexpr size_t MAX_NAME = 15;

  std::string           name;   // what the player called it, as the Load game page lists it
  std::filesystem::path path;   // the save
  std::array<std::array<char, SIZE>, SIZE> board;

  // The basic properties of the figure, which upgrades bought improve, and
  // of every white zombie, black one and red one, the same for every game
  // and so not saved.
  Properties figure{ .actions = 2, .lives = 1, .wounds = 1, .range = 3 };
  Properties zombie{ .actions = 2, .lives = 1, .wounds = 1 };
  Properties blackZombie{ .actions = 3, .lives = 2, .wounds = 1 };
  Properties redZombie{ .actions = 3, .lives = 3, .wounds = 2 };

  // In Kč: what every round on the goban has earned, added up, less what
  // has been spent.
  int money = 0;

  // The secret password -- typed into the line F3 opens -- and what it earns.
  static constexpr std::string_view SECRET_PASSWORD = "magorie1";
  static constexpr int              SECRET_REWARD   = 100000;

  // How many times each of UPGRADES has been bought.
  std::array<int, std::size(UPGRADES)> bought{};

  // What UPGRADES[i] costs now. Never nothing: it is never sold out.
  std::optional<int> price(size_t i) const;

  // Buys UPGRADES[i] for the figure, if it is still sold and the money is
  // there. False if not.
  bool buy(size_t i);

  // `typed` into the line F3 opens: the reward if it is the secret password.
  // False if it isn't -- and anything else does nothing.
  bool enterPassword(std::string_view typed);

  // The furthest wave any round has got to. Every wave a whole number of
  // CHECKPOINT_EVERY it has got to, a round can start at, straight from the
  // game's menu.
  int furthestWave = 1;
  static constexpr int CHECKPOINT_EVERY = 5;

  // The round being played, from the last Start.
  Goban goban;

  // A new game called `name`, with an empty board, given a save no other
  // game has.
  static Game New(std::string name);

  // The game in the save at `path`; nothing if it can't be read. Squares the
  // save doesn't have, or has something unknown on, are empty.
  static std::optional<Game> Load(const std::filesystem::path& path);

  // Writes the game to its save, with the time it was saved. False if it
  // couldn't be written.
  bool save() const;
};

// A save, as the Load game page lists it.
struct SavedGame {
  std::filesystem::path path;
  std::string           name;
  std::time_t           saved = 0;  // when it was last saved
};

// Every save there is, the last saved first.
std::vector<SavedGame> ListSavedGames();

// Deletes the save at `path` for good. False if it couldn't be.
bool DeleteSave(const std::filesystem::path& path);

// `time` as the Load game page shows it, in local time: "29. 9. 2026 15:30".
std::string SaveTimeText(std::time_t time);

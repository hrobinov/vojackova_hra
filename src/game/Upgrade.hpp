// What the shop in the game's menu sells: upgrades of the figure, and its
// special abilities. Each costs its prices, one after another, each dearer --
// and after the last, twice what it cost before, every time, for ever.

#pragma once

#include <game/Goban.hpp>

#include <span>

struct Upgrade {
  const char*          key;     // what the save calls it
  const char*          name;    // as the shop shows it
  std::span<const int> prices;  // in Kč: the first time, the second, ...
  void (*apply)(Properties& figure);
  bool                 special = false;  // a special ability, listed apart
};

inline constexpr int ACTION_PRICES[] = { 50, 100, 200 };
inline constexpr int LIFE_PRICES[]   = { 20, 70, 150, 250 };
inline constexpr int WOUND_PRICES[]  = { 300 };
inline constexpr int ROCKET_PRICES[] = { 20, 30, 60, 100 };
inline constexpr int SHELL_PRICES[]  = { 20, 30, 50 };

// In the order the shop shows them: the upgrades from the top left along the
// top, the special abilities down the left under them.
inline constexpr Upgrade UPGRADES[] = {
  { "actions", "+1 akce", ACTION_PRICES, [](Properties& figure) { ++figure.actions; } },
  { "lives", "+1 život", LIFE_PRICES, [](Properties& figure) { ++figure.lives; } },
  { "wounds", "+1 zranění", WOUND_PRICES, [](Properties& figure) { ++figure.wounds; } },
  // A rocket more every round: they are all back at every Start.
  { "rockets", "Raketa", ROCKET_PRICES, [](Properties& figure) { ++figure.rockets; }, true },
  // A shotgun shell more every round, the same way.
  { "shells", "Brokovnice", SHELL_PRICES, [](Properties& figure) { ++figure.shells; }, true },
};

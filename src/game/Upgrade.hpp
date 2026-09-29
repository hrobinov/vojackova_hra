// What the shop in the game's menu sells: upgrades of the figure. Each can be
// bought as many times as it has prices, one after another, each dearer --
// and after the last, never again.

#pragma once

#include <game/Goban.hpp>

#include <span>

struct Upgrade {
  const char*          key;     // what the save calls it
  const char*          name;    // as the shop shows it
  std::span<const int> prices;  // in Kč: the first time, the second, ...
  void (*apply)(Properties& figure);
};

inline constexpr int ACTION_PRICES[] = { 50, 100, 200 };
inline constexpr int LIFE_PRICES[]   = { 20, 70, 150, 250 };
inline constexpr int WOUND_PRICES[]  = { 300 };

// In the order the shop shows them, from the top left along the top.
inline constexpr Upgrade UPGRADES[] = {
  { "actions", "+1 akce", ACTION_PRICES, [](Properties& figure) { ++figure.actions; } },
  { "lives", "+1 život", LIFE_PRICES, [](Properties& figure) { ++figure.lives; } },
  { "wounds", "+1 zranění", WOUND_PRICES, [](Properties& figure) { ++figure.wounds; } },
};

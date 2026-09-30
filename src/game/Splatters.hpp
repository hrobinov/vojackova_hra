// What is left on the goban where someone dies: the zombie's body falling
// and lying there, and a puddle running out from under it -- a pool, and
// streams wandering off it, spreading as the liquid flows, each ending in a
// drop -- then all of it fading into the grass. Green from a white zombie,
// black from a black one, red from the soldier (whose body the goban draws).
//
// Only a picture, drawn under everyone on the goban; positions in squares,
// so it stays put when the window is resized.

#pragma once

#include <game/BoardView.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <optional>
#include <random>
#include <vector>

class Splatters {
public:
  // What each bleeds.
  static constexpr Color WHITE_ZOMBIE = Color{ 92, 170, 40, 255 };
  static constexpr Color BLACK_ZOMBIE = Color{ 22, 18, 24, 255 };
  static constexpr Color SOLDIER      = Color{ 105, 4, 10, 255 };

  // A zombie has died, facing `facing`: its body falls, and it bleeds.
  void addBody(const Goban::Zombie& zombie, Vector2 facing);
  // The soldier has died on the square `at`: he bleeds.
  void add(Position at, Color color);

  // Time goes on, by `seconds`: bodies fall, puddles spread, and in time
  // they fade and are gone.
  void update(float seconds);

  void draw(const BoardLayout& goban) const;

  void clear() { this->puddles.clear(); }

private:
  // A blob of the puddle, in squares from the middle of the square; it
  // spreads once the liquid has flowed that far.
  struct Blob {
    float x, y, r;
    float delay;
  };
  struct Body {
    Goban::Zombie zombie;
    Vector2       facing;
  };
  struct Puddle {
    Position            at;
    Color               color;
    float               age = 0.0f;
    std::vector<Blob>   blobs;
    std::optional<Body> body;
  };

  std::vector<Puddle> puddles;
  std::mt19937        random{ std::random_device{}() };
};

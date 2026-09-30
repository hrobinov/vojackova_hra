// What is left on the goban where someone dies: the zombie's body falling
// and lying there, and a puddle running out from under it -- a pool, and
// streams wandering off it, spreading as the liquid flows, each ending in a
// drop. Red from a white zombie, black from a black one, from a red one
// red so dark it is near black, streaked black; red from the soldier (whose
// body the goban draws). They lie there till the wave is
// over, and fade into the grass as the next comes.
//
// A zombie shot sprays blood away from the shot, behind it; one blown up by
// a rocket bleeds a much bigger puddle, splashed out at once before it
// even starts to flow, and sprays it all round, away from where the rocket
// hit.
//
// And a zombie killed by a shot may spurt: a jet of blood flying out
// behind it, landing in a heap on the square behind, there at once.
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
  static constexpr Color WHITE_ZOMBIE = Color{ 124, 10, 12, 255 };
  static constexpr Color BLACK_ZOMBIE = Color{ 22, 18, 24, 255 };
  static constexpr Color RED_ZOMBIE   = Color{ 62, 4, 8, 255 };
  static constexpr Color SOLDIER      = Color{ 105, 4, 10, 255 };

  // A zombie has died, facing `facing`: its body falls, and it bleeds --
  // blown up by a rocket that hit `blast`, or shot.
  void addBody(const Goban::Zombie& zombie, Vector2 facing, std::optional<Position> blast = std::nullopt);
  // The soldier has died on the square `at`: he bleeds.
  void add(Position at, Color color) { this->add(at, color, 1.0f); }

  // Time goes on, by `seconds`: bodies fall, puddles spread; those fading
  // fade, and are gone.
  void update(float seconds);

  // Everything there now starts to fade, `after` seconds from now: the wave
  // they fell in is over.
  void fadeAll(float after);

  void draw(const BoardLayout& goban) const;

  void clear()
  {
    this->puddles.clear();
    this->jets.clear();
  }

private:
  // A puddle `size` times the usual.
  void add(Position at, Color color, float size);
  // Blood sprayed off the square `at` the way `away` points (a unit long),
  // `wide` radians either side of it: `drops` drops, as far as `reach`.
  void spray(Vector2 away, float wide, int drops, float reach);

  // A blob of the puddle, in squares from the middle of the square; it
  // spreads once the liquid has flowed that far.
  struct Blob {
    float x, y, r;
    float delay;
    bool  sudden = false;  // there all at once when its time comes, not spreading
    bool  streak = false;  // the puddle's streak colour, not its own
  };
  // Blood flying from `from` to `to`, squares on the goban; each drop off
  // by `delay`, to the side by `aside`, `r` big.
  struct Drop {
    float delay, aside, r;
  };
  struct Jet {
    Vector2           from, to;
    Color             color;
    float             age = 0.0f;
    std::vector<Drop> drops;
  };
  struct Body {
    Goban::Zombie zombie;
    Vector2       facing;
  };
  struct Puddle {
    Position            at;
    Color               color;
    Color               streaks{};  // some of it this colour
    float               age = 0.0f;
    float               fadeAt = -1.0f;  // how old it starts to fade, if it is to
    std::vector<Blob>   blobs;
    std::optional<Body> body;
  };

  std::vector<Puddle> puddles;
  std::vector<Jet>    jets;
  std::mt19937        random{ std::random_device{}() };
};

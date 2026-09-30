// Rockets and shotgun blasts going off on the goban, drawn over it.
//
// A rocket first flies from the figure to the square it was fired at -- a
// green rocket, its exhaust burning and a trail of smoke behind it -- and
// only then goes off; the zombies it kills are still there until it does.
// Its blast is over the square it hit and the eight round it: every
// square scorched and a fireball on it, a moment apart so it doesn't look
// stamped out, and a big billowing one over them all; a flash and a
// shockwave from the middle; sparks flying out, and smoke rising after. The
// goban shakes as it goes off.
//
// A shotgun's is smaller: a flash at the muzzle, sparks sprayed the way it
// was fired, a smaller fireball and scorch on each of the three squares it
// hit, a wisp of smoke, and a jolt.
//
// Only a picture: what was done is already done (Goban::fireRocket,
// Goban::fireShotgun). Positions are kept in squares, so a blast stays put
// when the window is resized.

#pragma once

#include <game/BoardView.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <array>
#include <random>
#include <vector>

class Explosions {
public:
  // A rocket fired from the square `from` at the square `at`, where it
  // kills the zombies on `doomed` -- shown till it lands.
  void addRocket(Position from, Position at, std::vector<Goban::Zombie> doomed);

  // How long a rocket fired from `from` takes to land on `at`, in seconds.
  static float FlightTime(Position from, Position at);

  // The figure on `from` has fired the shotgun, hitting `hit`.
  void addShotgun(Position from, const std::array<Position, 3>& hit);

  // Time goes on, by `seconds`: blasts grow, fade, and are gone.
  void update(float seconds);

  // Over the goban laid out as `goban`.
  void draw(const BoardLayout& goban) const;

  // How far to move the goban to shake it, in pixels: most as something goes
  // off, and dying away.
  Vector2 shake(const BoardLayout& goban) const;

  void clear() { this->blasts.clear(); }

private:
  // In squares, from the middle of the blast's heart; speeds in squares a
  // second.
  struct Spark {
    float x, y, vx, vy;
    float life, lived = 0.0f;
  };
  struct Puff {
    float x, y;
    float delay, life;
  };
  struct Blast {
    bool                  rocket = true;  // a rocket's, or a shotgun's
    Position              heart;          // the square hit, or the shotgun's muzzle
    Position              from;           // where a rocket was fired from
    float                 flight = 0.0f;  // how long a rocket flies, before the blast
    std::vector<Goban::Zombie> doomed;    // zombies the rocket kills, till it lands
    std::vector<Puff>     trail;          // the smoke the rocket leaves as it flies
    float                 sinceTrail = 0.0f;
    Vector2               aim{};          // the shotgun: which way it fired, a unit long
    std::vector<Position> squares;        // the squares it hit
    std::vector<float>    delays;         // when each square's fireball starts
    float                 billows[6]{};   // which ways a rocket's big fireball bulges
    float                 age = 0.0f;
    std::vector<Spark>    sparks;
    std::vector<Puff>     smoke;
  };

  std::vector<Blast> blasts;
  std::mt19937       random{ std::random_device{}() };
};

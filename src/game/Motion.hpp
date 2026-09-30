// Where the soldier and the zombies are seen to be, which is not quite where
// they are: they glide from square to square rather than jump -- but for a
// jump of more than a square or so, as the soldier back to the middle for a
// new wave -- and a zombie shambles, swaying from side to side as it goes and
// a little as it stands. Only for drawing; the goban has them on their
// squares.

#pragma once

#include <game/Goban.hpp>

#include <raylib.h>

#include <unordered_map>

class Motion {
public:
  // Time goes on, by `seconds`: everyone a little nearer where they are.
  void update(const Goban& goban, float seconds);

  // Where they are seen to be, in squares from the goban's top left square's
  // middle.
  Vector2 soldier() const { return this->figure; }
  Vector2 zombie(const Goban::Zombie& zombie) const;

  // How far a zombie leans off the way it faces, swaying: radians.
  float sway(const Goban::Zombie& zombie) const;

  void clear();

private:
  struct Seen {
    Vector2 at;
    float   stride = 0.0f;  // how far into its shambling step, in radians
    float   moving = 0.0f;  // 0 standing, to 1 on the move
  };

  Vector2                            figure{ -100.0f, -100.0f };
  std::unordered_map<unsigned, Seen> zombies;
  float                              time = 0.0f;
};

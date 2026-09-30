// The game's sounds: recordings from resources\sounds (where each comes
// from is in CREDITS.txt there), carried in the exe. Where there are several
// takes of a sound, each time one of them, a little differently pitched, so
// it doesn't sound the same over and over.
//
// Needs the audio device (InitAudioDevice) for as long as it lives.

#pragma once

#include <game/Goban.hpp>

#include <raylib.h>

#include <array>
#include <random>
#include <vector>

class Sounds {
public:
  Sounds();
  ~Sounds();
  Sounds(const Sounds&) = delete;
  Sounds& operator=(const Sounds&) = delete;

  void zombieDies(Goban::Kind kind);
  void soldierDies();
  void soldierHit();  // a zombie's blow
  void pistol();
  void shotgun();
  void explosion();

private:
  // The takes of a sound, each with copies of itself, so several can play
  // at once.
  struct Take {
    Sound                sound{};
    std::array<Sound, 3> copies{};
    size_t               next = 0;
  };
  using Takes = std::vector<Take>;

  void play(Takes& takes, float pitch, float wobble);

  Takes whiteZombie, blackZombie, soldier, punches, pistolShots, shotgunShots, explosions;
  std::mt19937 random{ std::random_device{}() };
};

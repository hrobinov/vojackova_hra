#include <game/Sounds.hpp>

#include <cstddef>
#include <initializer_list>
#include <utility>

// Embedded by tools/Embed.cpp: each `name[]`, and `name_SIZE` bytes of it.
#define VH_SOUNDS(X)                                                                                                   \
  X(white_zombie_1) X(white_zombie_2) X(white_zombie_3) X(white_zombie_4) X(black_zombie_1) X(black_zombie_2)          \
  X(black_zombie_3) X(soldier_1) X(soldier_2) X(soldier_3) X(explosion) X(pistol) X(shotgun) X(punch_1) X(punch_2)    \
  X(punch_3)

namespace sounds {
#define VH_DECLARE(name)                  \
  extern const unsigned char name[];      \
  extern const std::size_t   name##_SIZE;
VH_SOUNDS(VH_DECLARE)
#undef VH_DECLARE
}  // namespace sounds

namespace {

struct Embedded {
  const unsigned char* data;
  std::size_t          size;
};
#define VH_EMBEDDED(name) Embedded{ sounds::name, sounds::name##_SIZE }

// How loud each is, against the others.
constexpr float GROAN_VOLUME = 0.8f;
constexpr float CRY_VOLUME   = 0.9f;
constexpr float PUNCH_VOLUME = 0.8f;
constexpr float SHOT_VOLUME  = 0.3f;  // shots, softer than the rest
constexpr float BOOM_VOLUME  = 1.0f;

}  // namespace

Sounds::Sounds()
{
  const auto load = [](Takes& takes, std::initializer_list<Embedded> files, float volume) {
    for (const Embedded& file : files) {
      Wave  wave  = LoadWaveFromMemory(".wav", file.data, int(file.size));
      Take& take  = takes.emplace_back();
      take.sound  = LoadSoundFromWave(wave);
      UnloadWave(wave);
      SetSoundVolume(take.sound, volume);
      for (Sound& copy : take.copies) {
        copy = LoadSoundAlias(take.sound);
        SetSoundVolume(copy, volume);
      }
    }
  };
  load(this->whiteZombie,
       { VH_EMBEDDED(white_zombie_1), VH_EMBEDDED(white_zombie_2), VH_EMBEDDED(white_zombie_3), VH_EMBEDDED(white_zombie_4) },
       GROAN_VOLUME);
  load(this->blackZombie, { VH_EMBEDDED(black_zombie_1), VH_EMBEDDED(black_zombie_2), VH_EMBEDDED(black_zombie_3) },
       GROAN_VOLUME);
  load(this->soldier, { VH_EMBEDDED(soldier_1), VH_EMBEDDED(soldier_2), VH_EMBEDDED(soldier_3) }, CRY_VOLUME);
  load(this->punches, { VH_EMBEDDED(punch_1), VH_EMBEDDED(punch_2), VH_EMBEDDED(punch_3) }, PUNCH_VOLUME);
  load(this->pistolShots, { VH_EMBEDDED(pistol) }, SHOT_VOLUME);
  load(this->shotgunShots, { VH_EMBEDDED(shotgun) }, SHOT_VOLUME);
  load(this->explosions, { VH_EMBEDDED(explosion) }, BOOM_VOLUME);
}

Sounds::~Sounds()
{
  for (Takes* takes : { &this->whiteZombie, &this->blackZombie, &this->soldier, &this->punches, &this->pistolShots,
                        &this->shotgunShots, &this->explosions }) {
    for (Take& take : *takes) {
      for (Sound& copy : take.copies) UnloadSoundAlias(copy);
      UnloadSound(take.sound);
    }
  }
}

void Sounds::play(Takes& takes, float pitch, float wobble)
{
  if (takes.empty()) return;
  std::uniform_int_distribution<size_t> which(0, takes.size() - 1);
  std::uniform_real_distribution<float> by(-wobble, wobble);
  Take&  take = takes[which(this->random)];
  Sound& copy = take.copies[take.next];
  take.next   = (take.next + 1) % take.copies.size();
  SetSoundPitch(copy, pitch + by(this->random));
  PlaySound(copy);
}

void Sounds::zombieDies(Goban::Kind kind)
{
  // A black zombie's groan deeper, a red one's deeper still.
  if (kind == Goban::Kind::Red) this->play(this->blackZombie, 0.7f, 0.04f);
  else if (kind == Goban::Kind::Black) this->play(this->blackZombie, 0.85f, 0.05f);
  else                            this->play(this->whiteZombie, 1.0f, 0.08f);
}

void Sounds::soldierDies() { this->play(this->soldier, 1.0f, 0.03f); }
void Sounds::soldierHit() { this->play(this->punches, 1.0f, 0.1f); }
void Sounds::pistol() { this->play(this->pistolShots, 1.0f, 0.05f); }
void Sounds::shotgun() { this->play(this->shotgunShots, 1.0f, 0.05f); }
void Sounds::explosion() { this->play(this->explosions, 1.0f, 0.08f); }

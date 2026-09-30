#include <game/Splatters.hpp>

#include <algorithm>
#include <cmath>
#include <tuple>

namespace {

// How long a body takes to fall, a blob to spread once the liquid gets to
// it, and everything to fade away, in seconds.
constexpr float FALL   = 0.35f;
constexpr float SPREAD = 0.3f;
constexpr float FADE   = 1.0f;

// A blob there at once, not spreading: a rocket's splash.
constexpr float AT_ONCE = -1.0f;

// The puddle: its pool, and the streams running off it -- how many, how
// many steps each, how far a step, and how long the liquid takes over one.
constexpr int   POOL          = 4;
constexpr int   STREAMS_LEAST = 3;
constexpr int   STREAMS_MOST  = 5;
constexpr int   STEPS         = 12;
constexpr float STEP          = 0.075f;
constexpr float STEP_TIME     = 0.06f;
// Blown up by a rocket: how much bigger the puddle; how many drops sprayed,
// and how far, in squares. Shot: the same.
constexpr float BLASTED     = 2.2f;
constexpr int   SPLASHED    = 26;
constexpr int   BLAST_DROPS = 60;
constexpr float BLAST_REACH = 1.4f;
constexpr int   SHOT_DROPS  = 14;
constexpr float SHOT_REACH  = 0.7f;
// How much of a red zombie's blood is black, in 100.
constexpr int   STREAKED     = 30;
// A spurt: how likely, in 100, for a zombie shot; how long its jet flies;
// how many drops fly, and how many blobs the heap it lands in has.
constexpr int   SPURT_CHANCE = 25;
constexpr float JET_TIME     = 0.28f;
constexpr int   JET_DROPS    = 18;
constexpr int   HEAP         = 24;
// Under a body, the liquid only comes out as it lands.
constexpr float BLEED_AFTER   = 0.25f;

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

Color Shade(Color color, float by, float alpha)
{
  return Color{ Byte(color.r * by), Byte(color.g * by), Byte(color.b * by), Byte(alpha * 255.0f) };
}

float EaseOut(float t)
{
  t = std::clamp(t, 0.0f, 1.0f);
  return 1.0f - (1.0f - t) * (1.0f - t);
}

// How much of it is left, fading once it has begun to.
float Left(float age, float fadeAt)
{
  if (fadeAt < 0.0f) return 1.0f;
  return std::clamp((fadeAt + FADE - age) / FADE, 0.0f, 1.0f);
}

}  // namespace

void Splatters::addBody(const Goban::Zombie& zombie, Vector2 facing, std::optional<Position> blast)
{
  const Color blood = zombie.kind == Goban::Kind::Red ? RED_ZOMBIE : zombie.kind == Goban::Kind::Black ? BLACK_ZOMBIE : WHITE_ZOMBIE;
  if (blast) {
    // Blown up: a big pool, and blood flung all round, away from the blast
    // -- or everywhere, right where it hit.
    this->add(zombie.at, blood, BLASTED);
    // Splashed out at once, all round it.
    Puddle&                               splashed = this->puddles.back();
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    for (int i = 0; i < SPLASHED; ++i) {
      const float angle = 2.0f * PI * unit(this->random), out = 0.95f * std::sqrt(unit(this->random));
      splashed.blobs.push_back({ std::cos(angle) * out, std::sin(angle) * out, 0.08f + 0.16f * unit(this->random) * (1.0f - out * 0.5f), AT_ONCE });
    }
    const float dx = float(zombie.at.x - blast->x), dy = float(zombie.at.y - blast->y), d = std::hypot(dx, dy);
    if (d == 0.0f) this->spray({ 1.0f, 0.0f }, PI, BLAST_DROPS, BLAST_REACH);
    else           this->spray({ dx / d, dy / d }, 1.3f, BLAST_DROPS, BLAST_REACH);
  } else {
    // Shot: blood sprayed out behind it, away from the soldier it faced.
    this->add(zombie.at, blood, 1.0f);
    this->spray({ -facing.x, -facing.y }, 0.6f, SHOT_DROPS, SHOT_REACH);
  }
  Puddle& puddle = this->puddles.back();
  puddle.body    = Body{ zombie, facing };
  for (Blob& blob : puddle.blobs) {
    if (blob.delay != AT_ONCE) blob.delay += BLEED_AFTER;
  }
  // A red zombie's blood streaked black.
  if (zombie.kind == Goban::Kind::Red) {
    puddle.streaks = BLACK_ZOMBIE;
    std::uniform_int_distribution<int> streaked(0, 99);
    for (Blob& blob : puddle.blobs) blob.streak = streaked(this->random) < STREAKED;
  }

  // Now and then, shot, it spurts: a jet of blood out behind it, landing in
  // a heap on the square behind.
  std::uniform_int_distribution<int> percent(0, 99);
  if (blast || percent(this->random) >= SPURT_CHANCE) return;
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  const Vector2 behind = { -facing.x, -facing.y };
  Jet jet{ { float(zombie.at.x), float(zombie.at.y) }, { float(zombie.at.x) + behind.x, float(zombie.at.y) + behind.y }, blood };
  for (int i = 0; i < JET_DROPS; ++i) jet.drops.push_back({ 0.1f * unit(this->random), 0.5f * (unit(this->random) - 0.5f), 0.045f + 0.07f * unit(this->random) });
  this->jets.push_back(std::move(jet));
  for (int i = 0; i < HEAP; ++i) {
    const float angle = 2.0f * PI * unit(this->random), out = 0.48f * std::sqrt(unit(this->random));
    puddle.blobs.push_back({ behind.x + std::cos(angle) * out, behind.y + std::sin(angle) * out,
                             0.08f + 0.16f * unit(this->random) * (1.0f - out), JET_TIME + 0.04f * unit(this->random), true });
  }
}

void Splatters::fadeAll(float after)
{
  for (Puddle& puddle : this->puddles) {
    if (puddle.fadeAt < 0.0f) puddle.fadeAt = puddle.age + after;
  }
}

void Splatters::spray(Vector2 away, float wide, int drops, float reach)
{
  Puddle&                               puddle = this->puddles.back();
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  const float                           towards = std::atan2(away.y, away.x);
  for (int i = 0; i < drops; ++i) {
    // Flung out, the far ones smaller and later; each drawn out along the
    // way it flew, a streak ending in a drop.
    const float angle = towards + (2.0f * unit(this->random) - 1.0f) * wide;
    const float far   = 0.25f + reach * unit(this->random) * unit(this->random) + 0.1f * unit(this->random);
    const float r     = 0.02f + 0.06f * unit(this->random) * (1.0f - far / (reach + 0.4f));
    const float cx = std::cos(angle), cy = std::sin(angle);
    const float when  = 0.02f * far;
    for (int k = 0; k < 4; ++k) {
      const float back = float(k) * r * 1.3f;
      puddle.blobs.push_back({ cx * (far - back), cy * (far - back), r * (1.0f - 0.18f * float(k)), when });
    }
  }
}

void Splatters::add(Position at, Color color, float size)
{
  Puddle puddle;
  puddle.at    = at;
  puddle.color = color;
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);

  // The pool, round the middle, first.
  for (int i = 0; i < POOL; ++i) {
    const float angle = 2.0f * PI * unit(this->random);
    const float out   = 0.08f * unit(this->random);
    puddle.blobs.push_back({ std::cos(angle) * out * size, std::sin(angle) * out * size, (0.2f + 0.08f * unit(this->random)) * size, 0.0f });
  }
  // Streams running off it, wandering as they go, thinner the further they
  // get, and a drop gathering at the end of each.
  const int streams = int(float(STREAMS_LEAST + int(unit(this->random) * float(STREAMS_MOST - STREAMS_LEAST + 1))) * size);
  for (int s = 0; s < streams; ++s) {
    float       angle = 2.0f * PI * (float(s) + unit(this->random) * 0.7f) / float(streams);
    float       x = std::cos(angle) * 0.1f, y = std::sin(angle) * 0.1f;
    const int   steps = int(float(STEPS / 2 + int(unit(this->random) * float(STEPS / 2 + 1))) * size);
    const float wide  = (0.09f + 0.05f * unit(this->random)) * std::sqrt(size);
    for (int i = 0; i < steps; ++i) {
      angle += (unit(this->random) - 0.5f) * 0.9f;
      x += std::cos(angle) * STEP;
      y += std::sin(angle) * STEP;
      const float thin = 1.0f - 0.6f * float(i) / float(steps);
      puddle.blobs.push_back({ x, y, wide * thin, 0.08f + STEP_TIME * float(i) });
    }
    puddle.blobs.push_back({ x + std::cos(angle) * STEP * 0.6f, y + std::sin(angle) * STEP * 0.6f, wide * 0.75f,
                             0.08f + STEP_TIME * float(steps) });
  }
  // A few splashes, flung off as it hit the ground.
  for (int i = 0; i < 4; ++i) {
    const float angle = 2.0f * PI * unit(this->random);
    const float out   = 0.35f + 0.2f * unit(this->random);
    puddle.blobs.push_back({ std::cos(angle) * out, std::sin(angle) * out, 0.02f + 0.02f * unit(this->random),
                             0.02f * unit(this->random) });
  }
  this->puddles.push_back(std::move(puddle));
}

void Splatters::update(float seconds)
{
  for (Puddle& puddle : this->puddles) puddle.age += seconds;
  for (Jet& jet : this->jets) jet.age += seconds;
  std::erase_if(this->jets, [](const Jet& jet) { return jet.age > JET_TIME + 0.2f; });
  std::erase_if(this->puddles, [](const Puddle& puddle) { return puddle.fadeAt >= 0.0f && puddle.age > puddle.fadeAt + FADE; });
}

void Splatters::draw(const BoardLayout& goban) const
{
  const float square = float(goban.square);
  const float middle = (square + float(goban.line)) / 2.0f;
  for (const Puddle& puddle : this->puddles) {
    const Vector2 at   = { float(goban.left) + float(puddle.at.x) * square + middle,
                           float(goban.top) + float(puddle.at.y) * square + middle };
    const float   left = Left(puddle.age, puddle.fadeAt);
    // A darker edge under the whole puddle, the liquid over it, and the
    // light shining off the wider parts of it.
    for (const auto& [by, grow, alpha, shine] : { std::tuple{ 0.55f, 1.15f, 0.85f, false }, std::tuple{ 1.0f, 1.0f, 0.85f, false },
                                                  std::tuple{ 1.6f, 0.35f, 0.3f, true } }) {
      for (const Blob& blob : puddle.blobs) {
        if (shine && blob.r < 0.1f) continue;
        const float spread = blob.delay == AT_ONCE ? 1.0f
                           : blob.sudden         ? (puddle.age >= blob.delay ? 1.0f : 0.0f)
                                                 : EaseOut((puddle.age - blob.delay) / SPREAD);
        if (spread <= 0.0f) continue;
        const float   r = blob.r * spread * grow * square;
        const Vector2 c = { at.x + blob.x * square - (shine ? r * 0.8f : 0.0f), at.y + blob.y * square - (shine ? r * 0.8f : 0.0f) };
        DrawCircleV(c, r, Shade(blob.streak ? puddle.streaks : puddle.color, by, alpha * left));
      }
    }
    // The body in it, falling, and fading with it.
    if (puddle.body) {
      DrawZombie(at, square, puddle.body->facing, puddle.body->zombie, std::min(1.0f, puddle.age / FALL), left);
    }
  }

  // Jets of blood in flight, over everything: each drop a streak along its
  // way, bigger mid-flight as it arcs up towards the eye.
  const auto onGoban = [&](Vector2 at) {
    return Vector2{ float(goban.left) + at.x * square + middle, float(goban.top) + at.y * square + middle };
  };
  for (const Jet& jet : this->jets) {
    const Vector2 way  = { jet.to.x - jet.from.x, jet.to.y - jet.from.y };
    const Vector2 side = { -way.y, way.x };
    for (const Drop& drop : jet.drops) {
      const float t = (jet.age - drop.delay) / JET_TIME;
      if (t < 0.0f || t > 1.0f) continue;
      const auto    along = [&](float p) {
        return onGoban({ jet.from.x + way.x * p + side.x * drop.aside * p, jet.from.y + way.y * p + side.y * drop.aside * p });
      };
      const float   arc  = 1.0f + 1.3f * std::sin(PI * t);
      const Vector2 head = along(t), tail = along(std::max(0.0f, t - 0.18f));
      DrawLineEx(tail, head, drop.r * square * arc, Shade(jet.color, 0.8f, 0.9f));
      DrawCircleV(head, drop.r * square * arc * 0.65f, Shade(jet.color, 1.0f, 0.95f));
    }
  }
}

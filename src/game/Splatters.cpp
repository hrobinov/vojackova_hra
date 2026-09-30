#include <game/Splatters.hpp>

#include <algorithm>
#include <cmath>
#include <tuple>

namespace {

// How long a body takes to fall, a blob to spread once the liquid gets to
// it, everything to stay, and then to fade away, in seconds.
constexpr float FALL   = 0.35f;
constexpr float SPREAD = 0.3f;
constexpr float STAY   = 3.0f;
constexpr float FADE   = 1.0f;

// The puddle: its pool, and the streams running off it -- how many, how
// many steps each, how far a step, and how long the liquid takes over one.
constexpr int   POOL          = 4;
constexpr int   STREAMS_LEAST = 3;
constexpr int   STREAMS_MOST  = 5;
constexpr int   STEPS         = 12;
constexpr float STEP          = 0.075f;
constexpr float STEP_TIME     = 0.06f;
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

// How much of it is left, fading out at the end.
float Left(float age)
{
  return std::clamp((FALL + STAY + FADE - age) / FADE, 0.0f, 1.0f);
}

}  // namespace

void Splatters::addBody(const Goban::Zombie& zombie, Vector2 facing)
{
  this->add(zombie.at, zombie.kind == Goban::Kind::Black ? BLACK_ZOMBIE : WHITE_ZOMBIE);
  Puddle& puddle = this->puddles.back();
  puddle.body    = Body{ zombie, facing };
  for (Blob& blob : puddle.blobs) blob.delay += BLEED_AFTER;
}

void Splatters::add(Position at, Color color)
{
  Puddle puddle;
  puddle.at    = at;
  puddle.color = color;
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);

  // The pool, round the middle, first.
  for (int i = 0; i < POOL; ++i) {
    const float angle = 2.0f * PI * unit(this->random);
    const float out   = 0.08f * unit(this->random);
    puddle.blobs.push_back({ std::cos(angle) * out, std::sin(angle) * out, 0.2f + 0.08f * unit(this->random), 0.0f });
  }
  // Streams running off it, wandering as they go, thinner the further they
  // get, and a drop gathering at the end of each.
  const int streams = STREAMS_LEAST + int(unit(this->random) * float(STREAMS_MOST - STREAMS_LEAST + 1));
  for (int s = 0; s < streams; ++s) {
    float       angle = 2.0f * PI * (float(s) + unit(this->random) * 0.7f) / float(streams);
    float       x = std::cos(angle) * 0.1f, y = std::sin(angle) * 0.1f;
    const int   steps = STEPS / 2 + int(unit(this->random) * float(STEPS / 2 + 1));
    const float wide  = 0.09f + 0.05f * unit(this->random);
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
  std::erase_if(this->puddles, [](const Puddle& puddle) { return puddle.age > FALL + STAY + FADE; });
}

void Splatters::draw(const BoardLayout& goban) const
{
  const float square = float(goban.square);
  const float middle = (square + float(goban.line)) / 2.0f;
  for (const Puddle& puddle : this->puddles) {
    const Vector2 at   = { float(goban.left) + float(puddle.at.x) * square + middle,
                           float(goban.top) + float(puddle.at.y) * square + middle };
    const float   left = Left(puddle.age);
    // A darker edge under the whole puddle, the liquid over it, and the
    // light shining off the wider parts of it.
    for (const auto& [by, grow, alpha, shine] : { std::tuple{ 0.55f, 1.15f, 0.85f, false }, std::tuple{ 1.0f, 1.0f, 0.85f, false },
                                                  std::tuple{ 1.6f, 0.35f, 0.3f, true } }) {
      for (const Blob& blob : puddle.blobs) {
        if (shine && blob.r < 0.1f) continue;
        const float spread = EaseOut((puddle.age - blob.delay) / SPREAD);
        if (spread <= 0.0f) continue;
        const float   r = blob.r * spread * grow * square;
        const Vector2 c = { at.x + blob.x * square - (shine ? r * 0.8f : 0.0f), at.y + blob.y * square - (shine ? r * 0.8f : 0.0f) };
        DrawCircleV(c, r, Shade(puddle.color, by, alpha * left));
      }
    }
    // The body in it, falling, and fading with it.
    if (puddle.body) {
      DrawZombie(at, square, puddle.body->facing, puddle.body->zombie, std::min(1.0f, puddle.age / FALL), left);
    }
  }
}

#include <game/Explosions.hpp>

#include <algorithm>
#include <cmath>

namespace {

// How long each part lasts, in seconds, and when the blast is over.
constexpr float FIRE_TIME  = 0.6f;   // the fireball
constexpr float FLASH_TIME = 0.1f;   // the white flash in the middle
constexpr float WAVE_TIME  = 0.4f;   // the shockwave
constexpr float SHAKE_TIME = 0.3f;
constexpr float BLAST_TIME = 1.6f;   // the last of the smoke

// How much of it there is.
constexpr int   SPARKS     = 36;
constexpr int   PUFFS      = 12;
constexpr float SHAKE_SIZE = 0.18f;  // squares

// A shotgun's, smaller and quicker.
constexpr float SHOTGUN_FIRE_TIME  = 0.35f;
constexpr int   SHOTGUN_SPARKS     = 24;
constexpr int   SHOTGUN_PUFFS      = 4;
constexpr float SHOTGUN_SHAKE_SIZE = 0.07f;
constexpr float DRAG       = 3.0f;   // how fast sparks slow, a second
constexpr float GRAVITY    = 2.5f;   // squares a second a second

float EaseOut(float t)
{
  return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

// From `a` to `b`, as `t` goes from 0 to 1.
Color Mix(Color a, Color b, float t)
{
  t = std::clamp(t, 0.0f, 1.0f);
  return Color{ Byte(a.r + (b.r - a.r) * t), Byte(a.g + (b.g - a.g) * t), Byte(a.b + (b.b - a.b) * t),
                Byte(a.a + (b.a - a.a) * t) };
}

Color WithAlpha(Color color, float alpha)
{
  color.a = Byte(alpha * 255.0f);
  return color;
}

bool OnGoban(int x, int y)
{
  return x >= 0 && x < Goban::SIZE && y >= 0 && y < Goban::SIZE;
}

// The fire's colours, from the white-hot heart to the dark red of its edge.
constexpr Color FIRE_WHITE  = Color{ 255, 250, 230, 255 };
constexpr Color FIRE_YELLOW = Color{ 255, 210, 60, 255 };
constexpr Color FIRE_ORANGE = Color{ 250, 115, 15, 255 };
constexpr Color FIRE_RED    = Color{ 170, 30, 5, 255 };
constexpr Color SCORCH      = Color{ 45, 20, 8, 255 };
constexpr Color SMOKE       = Color{ 40, 36, 34, 255 };

// A ball of fire `t` of the way through its life: a dark red rim, orange, a
// yellow heart and a white-hot core, which cool and shrink as it burns out.
void Fireball(Vector2 at, float radius, float t, float alpha)
{
  const auto blob = [&](float r, Color inner, Color outer) {
    if (r >= 1.0f) DrawCircleGradient(int(at.x), int(at.y), r, inner, outer);
  };
  blob(radius, WithAlpha(Mix(FIRE_ORANGE, FIRE_RED, t), alpha * 0.9f), WithAlpha(FIRE_RED, 0.0f));
  blob(radius * 0.72f, WithAlpha(Mix(FIRE_YELLOW, FIRE_ORANGE, t * 1.5f), alpha), WithAlpha(FIRE_ORANGE, 0.0f));
  blob(radius * 0.45f * (1.0f - 0.6f * t), WithAlpha(Mix(FIRE_WHITE, FIRE_YELLOW, t * 2.0f), alpha),
       WithAlpha(FIRE_YELLOW, 0.0f));
}

}  // namespace

void Explosions::addRocket(Position at)
{
  Blast blast;
  blast.rocket = true;
  blast.heart  = at;

  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  // The middle first, the others a moment after, each a little differently.
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      blast.squares.push_back({ at.x + dx, at.y + dy });
      blast.delays.push_back(dx == 0 && dy == 0 ? 0.0f : 0.02f + 0.08f * unit(this->random));
    }
  }
  for (int i = 0; i < SPARKS; ++i) {
    const float angle = 2.0f * PI * unit(this->random);
    const float speed = 4.0f + 6.0f * unit(this->random);
    blast.sparks.push_back({ 0.0f, 0.0f, std::cos(angle) * speed, std::sin(angle) * speed, 0.3f + 0.45f * unit(this->random) });
  }
  for (int i = 0; i < PUFFS; ++i) {
    blast.smoke.push_back({ -1.3f + 2.6f * unit(this->random), -1.3f + 2.6f * unit(this->random), 0.12f + 0.25f * unit(this->random),
                            0.8f + 0.6f * unit(this->random) });
  }
  for (float& angle : blast.billows) angle = 2.0f * PI * unit(this->random);
  this->blasts.push_back(std::move(blast));
}

void Explosions::addShotgun(Position from, const std::array<Position, 3>& hit)
{
  Blast blast;
  blast.rocket = false;
  blast.heart  = from;
  // Which way it fired: at the middle one of the squares hit.
  const float ax = float(hit[1].x - from.x), ay = float(hit[1].y - from.y);
  const float length = std::sqrt(ax * ax + ay * ay);
  blast.aim = { ax / length, ay / length };

  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  // The pellets reach the middle square first, the sides a moment after.
  for (size_t i = 0; i < hit.size(); ++i) {
    blast.squares.push_back(hit[i]);
    blast.delays.push_back(i == 1 ? 0.03f : 0.05f + 0.03f * unit(this->random));
  }
  // A spray of sparks out of the muzzle, the way it fired: fast, and soon gone.
  const float aim = std::atan2(blast.aim.y, blast.aim.x);
  for (int i = 0; i < SHOTGUN_SPARKS; ++i) {
    const float angle = aim + (unit(this->random) - 0.5f) * 1.6f;
    const float speed = 8.0f + 8.0f * unit(this->random);
    blast.sparks.push_back({ blast.aim.x * 0.35f, blast.aim.y * 0.35f, std::cos(angle) * speed, std::sin(angle) * speed,
                             0.1f + 0.15f * unit(this->random) });
  }
  // A little smoke over where it hit.
  for (int i = 0; i < SHOTGUN_PUFFS; ++i) {
    blast.smoke.push_back({ blast.aim.x + (unit(this->random) - 0.5f) * 1.6f, blast.aim.y + (unit(this->random) - 0.5f) * 1.6f,
                            0.1f + 0.15f * unit(this->random), 0.6f + 0.4f * unit(this->random) });
  }
  this->blasts.push_back(std::move(blast));
}

void Explosions::update(float seconds)
{
  for (Blast& blast : this->blasts) {
    blast.age += seconds;
    for (Spark& spark : blast.sparks) {
      spark.lived += seconds;
      const float slow = std::exp(-DRAG * seconds);
      spark.vx *= slow;
      spark.vy = spark.vy * slow + GRAVITY * seconds;
      spark.x += spark.vx * seconds;
      spark.y += spark.vy * seconds;
    }
  }
  std::erase_if(this->blasts, [](const Blast& blast) { return blast.age > BLAST_TIME; });
}

Vector2 Explosions::shake(const BoardLayout& goban) const
{
  Vector2 offset{ 0.0f, 0.0f };
  for (const Blast& blast : this->blasts) {
    if (blast.age >= SHAKE_TIME) continue;
    const float size     = blast.rocket ? SHAKE_SIZE : SHOTGUN_SHAKE_SIZE;
    const float strength = (1.0f - blast.age / SHAKE_TIME) * size * float(goban.square);
    offset.x += std::sin(blast.age * 97.0f) * strength;
    offset.y += std::cos(blast.age * 71.0f) * strength;
  }
  return offset;
}

void Explosions::draw(const BoardLayout& goban) const
{
  const float square = float(goban.square);
  const float middle = (square + float(goban.line)) / 2.0f;
  // The middle of the square (x, y), in pixels.
  const auto centre = [&](Position at) {
    return Vector2{ float(goban.left) + float(at.x) * square + middle, float(goban.top) + float(at.y) * square + middle };
  };

  for (const Blast& blast : this->blasts) {
    const Vector2 heart = centre(blast.heart);

    // Scorched squares, darkening at once and then fading out.
    for (size_t i = 0; i < blast.squares.size(); ++i) {
      const Position at = blast.squares[i];
      if (!OnGoban(at.x, at.y)) continue;
      const float t = (blast.age - blast.delays[i]) / BLAST_TIME;
      if (t < 0.0f || t > 1.0f) continue;
      const float alpha = (blast.rocket ? 0.6f : 0.45f) * std::min(1.0f, t * 10.0f) * (1.0f - t);
      DrawRectangle(goban.left + at.x * goban.square + goban.line, goban.top + at.y * goban.square + goban.line,
                    goban.square - goban.line, goban.square - goban.line, WithAlpha(SCORCH, alpha));
    }

    // Smoke, rising and spreading after the fire.
    for (const Puff& puff : blast.smoke) {
      const float t = (blast.age - puff.delay) / puff.life;
      if (t < 0.0f || t > 1.0f) continue;
      const Vector2 at     = { heart.x + puff.x * square, heart.y + (puff.y - 1.0f * t) * square };
      const float   radius = square * (blast.rocket ? 0.45f + 0.7f * t : 0.3f + 0.5f * t);
      const float   alpha  = (blast.rocket ? 0.6f : 0.45f) * std::sin(PI * t);
      DrawCircleGradient(int(at.x), int(at.y), radius, WithAlpha(SMOKE, alpha), WithAlpha(SMOKE, 0.0f));
    }

    // The fire: a ball over each square hit, a moment apart -- and for a
    // rocket, one big one over them all, billowing out at the sides so it
    // isn't a plain circle.
    for (size_t i = 0; i < blast.squares.size(); ++i) {
      const Position at = blast.squares[i];
      if (!OnGoban(at.x, at.y)) continue;
      const float t = (blast.age - blast.delays[i]) / (blast.rocket ? FIRE_TIME : SHOTGUN_FIRE_TIME);
      if (t < 0.0f || t > 1.0f) continue;
      const float radius = square * (blast.rocket ? 0.55f + 0.6f * EaseOut(t) : 0.3f + 0.4f * EaseOut(t));
      Fireball(centre(at), radius, t, 1.0f - t * t);
    }
    if (const float t = blast.age / FIRE_TIME; blast.rocket && t <= 1.0f) {
      const float radius = square * (1.0f + 1.5f * EaseOut(t));
      const float alpha  = 1.0f - t * t;
      for (float angle : blast.billows) {
        const Vector2 at = { heart.x + std::cos(angle) * radius * 0.55f, heart.y + std::sin(angle) * radius * 0.55f };
        Fireball(at, radius * 0.55f, t, alpha * 0.8f);
      }
      Fireball(heart, radius, t, alpha);
    }

    // The flash, lighting up what is under it: where the rocket hits, or at
    // the shotgun's muzzle, on the figure's edge the way it fired.
    if (blast.age < FLASH_TIME) {
      const float   t     = blast.age / FLASH_TIME;
      const Vector2 at    = blast.rocket ? heart : Vector2{ heart.x + blast.aim.x * middle, heart.y + blast.aim.y * middle };
      const float   reach = blast.rocket ? 1.0f + 1.5f * t : 0.5f + 0.5f * t;
      BeginBlendMode(BLEND_ADDITIVE);
      DrawCircleGradient(int(at.x), int(at.y), square * reach, WithAlpha(FIRE_WHITE, 1.0f - t), WithAlpha(FIRE_YELLOW, 0.0f));
      EndBlendMode();
    }

    // A rocket's shockwave, running out past the squares it hit.
    if (blast.rocket && blast.age < WAVE_TIME) {
      const float t      = blast.age / WAVE_TIME;
      const float radius = square * (0.5f + 2.5f * EaseOut(t));
      const float width  = square * 0.18f * (1.0f - t) + 1.0f;
      DrawRing(heart, radius - width, radius, 0.0f, 360.0f, 72, WithAlpha(Mix(FIRE_WHITE, FIRE_ORANGE, t), 0.85f * (1.0f - t)));
    }

    // Sparks, each a streak along its way, yellow cooling to red, with a
    // brighter line down its middle.
    for (const Spark& spark : blast.sparks) {
      const float t = spark.lived / spark.life;
      if (t > 1.0f) continue;
      const Vector2 head  = { heart.x + spark.x * square, heart.y + spark.y * square };
      const Vector2 tail  = { head.x - spark.vx * square * 0.05f, head.y - spark.vy * square * 0.05f };
      const float   width = std::max(1.5f, square * (blast.rocket ? 0.09f : 0.06f) * (1.0f - 0.5f * t));
      DrawLineEx(tail, head, width, WithAlpha(Mix(FIRE_ORANGE, FIRE_RED, t), 1.0f - t));
      DrawLineEx(tail, head, width * 0.45f, WithAlpha(Mix(FIRE_WHITE, FIRE_YELLOW, t), 1.0f - t));
    }
  }
}

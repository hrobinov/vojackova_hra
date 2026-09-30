#include <game/Motion.hpp>

#include <algorithm>
#include <cmath>

namespace {

// How fast they glide, in squares a second; and past how far they jump.
constexpr float GLIDE = 6.0f;
constexpr float JUMP  = 1.6f;

// A zombie's shamble: how many steps a square, how far it leans with each,
// and how far it sways standing, in radians, and how fast.
constexpr float STEPS_A_SQUARE = 1.0f;
constexpr float STEP_LEAN      = 0.22f;
constexpr float IDLE_LEAN      = 0.07f;
constexpr float IDLE_SPEED     = 1.7f;

Vector2 Of(Position at)
{
  return { float(at.x), float(at.y) };
}

// `from` moved towards `to`, `by` at most; true if it moved.
bool Towards(Vector2& from, Vector2 to, float by)
{
  const float dx = to.x - from.x, dy = to.y - from.y, d = std::sqrt(dx * dx + dy * dy);
  if (d == 0.0f) return false;
  if (d > JUMP || d <= by) {
    from = to;
  } else {
    from.x += dx / d * by;
    from.y += dy / d * by;
  }
  return true;
}

}  // namespace

void Motion::update(const Goban& goban, float seconds)
{
  this->time += seconds;
  const float by = GLIDE * seconds;
  Towards(this->figure, Of(goban.figureAt()), by);

  // Those who are gone, forgotten; those who have come, where they are.
  std::erase_if(this->zombies, [&](const auto& seen) {
    return std::none_of(goban.zombies().begin(), goban.zombies().end(),
                        [&](const Goban::Zombie& z) { return z.id == seen.first; });
  });
  for (const Goban::Zombie& zombie : goban.zombies()) {
    auto [it, fresh] = this->zombies.try_emplace(zombie.id, Seen{ Of(zombie.at) });
    Seen&         seen   = it->second;
    const Vector2 before = seen.at;
    const bool    moved  = !fresh && Towards(seen.at, Of(zombie.at), by);
    const float   went   = std::hypot(seen.at.x - before.x, seen.at.y - before.y);
    seen.stride += went * STEPS_A_SQUARE * PI;
    seen.moving = std::clamp(seen.moving + (moved ? 6.0f : -3.0f) * seconds, 0.0f, 1.0f);
  }
}

Vector2 Motion::zombie(const Goban::Zombie& zombie) const
{
  const auto seen = this->zombies.find(zombie.id);
  return seen == this->zombies.end() ? Of(zombie.at) : seen->second.at;
}

float Motion::sway(const Goban::Zombie& zombie) const
{
  const auto seen = this->zombies.find(zombie.id);
  // Each its own: by its number, out of step with the others.
  const float own  = float(zombie.id % 97) * 1.37f;
  const float idle = IDLE_LEAN * std::sin(this->time * IDLE_SPEED + own);
  if (seen == this->zombies.end()) return idle;
  return idle * (1.0f - seen->second.moving) + STEP_LEAN * seen->second.moving * std::sin(seen->second.stride);
}

void Motion::clear()
{
  this->figure = { -100.0f, -100.0f };
  this->zombies.clear();
}

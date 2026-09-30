#include <game/BoardView.hpp>

#include <app/Config.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace {

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

Color WithAlpha(Color color, float alpha)
{
  color.a = Byte(alpha * 255.0f);
  return color;
}

Color Shade(Color color, float by)
{
  return Color{ Byte(color.r * by), Byte(color.g * by), Byte(color.b * by), color.a };
}

// Drawing someone seen from above, turned the way it faces: a point `ahead`
// squares in front of its middle and `aside` squares to its right.
struct Pose {
  Vector2 at;
  float   square;
  Vector2 front;  // a unit long, the way it faces

  Vector2 point(float ahead, float aside) const
  {
    const Vector2 right = { -this->front.y, this->front.x };
    return { this->at.x + (this->front.x * ahead + right.x * aside) * this->square,
             this->at.y + (this->front.y * ahead + right.y * aside) * this->square };
  }
  float angle() const { return std::atan2(this->front.y, this->front.x) * RAD2DEG; }

  void circle(float ahead, float aside, float radius, Color color) const
  {
    DrawCircleV(this->point(ahead, aside), radius * this->square, color);
  }
  // A bar from `from` to `to` squares ahead, `aside` to the right, `width` wide.
  void bar(float from, float to, float aside, float width, Color color) const
  {
    DrawLineEx(this->point(from, aside), this->point(to, aside), width * this->square, color);
  }
  // A block `length` along the way it faces and `width` across, its middle
  // `ahead` in front.
  void block(float ahead, float length, float width, Color color) const
  {
    const Vector2 c = this->point(ahead, 0.0f);
    DrawRectanglePro({ c.x, c.y, length * this->square, width * this->square },
                     { length * this->square / 2.0f, width * this->square / 2.0f }, this->angle(), color);
  }
};

// How big the soldier and the zombies are drawn, against the numbers that
// place their parts: a little over their squares' size, so they fill them.
constexpr float FIGURE_SCALE = 1.3f;

// The colours of the soldier.
constexpr Color UNIFORM      = Color{ 52, 104, 205, 255 };
constexpr Color HELMET       = Color{ 34, 66, 140, 255 };
constexpr Color PACK         = Color{ 92, 78, 52, 255 };
constexpr Color SKIN         = Color{ 236, 190, 150, 255 };
constexpr Color RIFLE        = Color{ 40, 38, 36, 255 };
constexpr Color RIFLE_STOCK  = Color{ 110, 72, 40, 255 };

// The colours of a white zombie: its pale, rotting skin, and the rags it
// wears, one of several.
constexpr Color ZOMBIE_SKIN  = Color{ 214, 224, 204, 255 };
constexpr Color ZOMBIE_ROT   = Color{ 150, 176, 120, 255 };
constexpr Color ZOMBIE_EYES  = Color{ 120, 20, 20, 255 };
constexpr Color RAGS[]       = { { 118, 102, 84, 255 }, { 86, 104, 92, 255 }, { 120, 84, 88, 255 }, { 92, 90, 120, 255 } };
// The dark line round a zombie.
constexpr Color OUTLINE      = Color{ 30, 26, 24, 255 };
// A black zombie's shirt and hair.
constexpr Color BLACK_SHIRT  = Color{ 34, 34, 40, 255 };
constexpr Color BLACK_HAIR   = Color{ 18, 16, 20, 255 };

// The badges' colours.
constexpr Color HEART        = Color{ 220, 36, 48, 255 };
constexpr Color BADGE_EDGE   = Color{ 40, 20, 20, 255 };
constexpr Color ACTIONS      = Color{ 250, 200, 40, 255 };

}  // namespace

Vector2 Facing(Position from, Position to)
{
  const float dx = float(to.x - from.x), dy = float(to.y - from.y);
  const float length = std::sqrt(dx * dx + dy * dy);
  if (length == 0.0f) return { 0.0f, -1.0f };
  return { dx / length, dy / length };
}

void DrawSoldier(Vector2 at, float square, Vector2 facing, float fallen)
{
  square *= FIGURE_SCALE;
  // Falling, he turns a quarter over and lands back the way he faced; lying
  // flat, his shadow is only just round him.
  const float   fall  = fallen * fallen;
  const float   turn  = fall * PI / 2.0f;
  const Vector2 front = { facing.x * std::cos(turn) - facing.y * std::sin(turn), facing.x * std::sin(turn) + facing.y * std::cos(turn) };
  const Vector2 down  = { at.x - facing.x * square * 0.18f * fall, at.y - facing.y * square * 0.18f * fall };
  const Pose    pose{ down, square, front };
  // Its shadow, cast down and to the right like the trees'.
  DrawCircleV({ down.x + square * 0.06f * (1.0f - fall), down.y + square * 0.08f * (1.0f - fall) }, square * 0.33f,
              WithAlpha(BLACK, 0.3f));
  // The pack on its back, the shoulders and the body between them.
  pose.block(-0.14f, 0.16f, 0.28f, PACK);
  pose.block(-0.16f, 0.06f, 0.24f, Shade(PACK, 0.8f));
  pose.circle(0.0f, -0.17f, 0.12f, UNIFORM);
  pose.circle(0.0f, 0.17f, 0.12f, UNIFORM);
  pose.block(0.0f, 0.22f, 0.34f, UNIFORM);
  // The arms reaching forward to the rifle, held out in front: the right
  // hand at the grip, the left along the barrel.
  pose.bar(0.02f, 0.18f, 0.17f, 0.09f, Shade(UNIFORM, 0.85f));
  pose.bar(0.02f, 0.3f, -0.15f, 0.09f, Shade(UNIFORM, 0.85f));
  pose.bar(-0.02f, 0.14f, 0.07f, 0.08f, RIFLE_STOCK);
  pose.bar(0.12f, 0.5f, 0.05f, 0.05f, RIFLE);
  pose.circle(0.18f, 0.12f, 0.045f, SKIN);
  pose.circle(0.3f, -0.07f, 0.045f, SKIN);
  // The helmet, the light catching it -- knocked off as he falls, and
  // rolling away, his hair showing under it.
  const float off = std::max(0.0f, fall - 0.4f) / 0.6f;
  if (off > 0.0f) pose.circle(0.0f, 0.0f, 0.11f, Color{ 90, 60, 36, 255 });
  pose.circle(0.0f, -0.38f * off, 0.15f, Shade(HELMET, 0.8f));
  pose.circle(0.0f, -0.38f * off, 0.13f, HELMET);
  pose.circle(0.03f, -0.04f - 0.38f * off, 0.05f, WithAlpha(Color{ 140, 170, 230, 255 }, 0.7f));
}

void DrawZombie(Vector2 at, float square, Vector2 facing, const Goban::Zombie& zombie, float fallen, float fade)
{
  const Position where = zombie.at;
  const bool     black = zombie.kind == Goban::Kind::Black;
  square *= FIGURE_SCALE;
  // Falling, it topples a quarter over and back, and its arms flop out.
  const float   fall  = fallen * fallen;
  const float   turn  = -fall * PI / 2.0f;
  const Vector2 front = { facing.x * std::cos(turn) - facing.y * std::sin(turn), facing.x * std::sin(turn) + facing.y * std::cos(turn) };
  const Vector2 down  = { at.x - facing.x * square * 0.15f * fall, at.y - facing.y * square * 0.15f * fall };
  const Pose    pose{ down, square, front };
  // Everything fainter as the body fades.
  const auto faded = [fade](Color color) { return WithAlpha(color, float(color.a) / 255.0f * fade); };

  // Each zombie a little different, the same one each time: by where it is.
  const unsigned look  = unsigned(where.x * 73856093) ^ unsigned(where.y * 19349663);
  const Color    rags  = black ? BLACK_SHIRT : RAGS[look % 4];
  const Color    skin  = ZOMBIE_SKIN;
  const float    spread = 0.15f + 0.1f * fall;  // the arms, apart
  const float    reach  = 0.36f - 0.12f * fall;  // and how far out in front

  DrawCircleV({ down.x + square * 0.06f * (1.0f - fall), down.y + square * 0.08f * (1.0f - fall) }, square * 0.32f,
              faded(WithAlpha(BLACK, 0.3f)));

  // Drawn twice: first a little bigger in a dark outline, then over it in
  // its colours, which makes it stand out on the grass like a cartoon's.
  for (const bool outline : { true, false }) {
    const float grow = outline ? 0.018f : 0.0f;
    const auto  paint = [&](Color color) { return faded(outline ? OUTLINE : color); };
    // The arms, stretched out in front grabbing: ragged sleeves, bare
    // forearms, and grasping fingers.
    for (const float side : { -spread, spread }) {
      pose.bar(0.0f, reach * 0.45f, side, 0.1f + 2 * grow, paint(Shade(rags, 0.9f)));
      pose.bar(reach * 0.4f, reach, side, 0.075f + 2 * grow, paint(Shade(skin, 0.9f)));
      pose.circle(reach + 0.02f, side, 0.05f + grow, paint(skin));
      for (const float finger : { -0.035f, 0.0f, 0.035f }) pose.circle(reach + 0.07f, side + finger, 0.018f + grow, paint(skin));
    }
    // Rags over hunched shoulders, torn at the back.
    pose.circle(-0.02f, -0.16f, 0.12f + grow, paint(rags));
    pose.circle(-0.02f, 0.16f, 0.12f + grow, paint(rags));
    pose.block(-0.03f, 0.2f + 2 * grow, 0.32f + 2 * grow, paint(rags));
    // The head, shaded round the back.
    pose.circle(0.03f, 0.0f, 0.135f + grow, paint(Shade(skin, 0.82f)));
    if (!outline) pose.circle(0.045f, -0.01f, 0.115f, faded(skin));
  }
  // Patches on the rags, and where they are torn through.
  pose.block(-0.1f, 0.05f, 0.2f, faded(Shade(rags, 0.75f)));
  pose.circle(-0.08f, (look & 4) ? -0.1f : 0.1f, 0.035f, faded(Shade(rags, 0.6f)));
  pose.circle(-0.04f, (look & 8) ? 0.19f : -0.19f, 0.03f, faded(Shade(skin, 0.85f)));
  // Rotting patches on the head.
  pose.circle(-0.02f, (look & 1) ? 0.05f : -0.05f, 0.045f, faded(ZOMBIE_ROT));
  pose.circle(0.04f, (look & 2) ? -0.07f : 0.07f, 0.028f, faded(ZOMBIE_ROT));
  // A black zombie's hair, over the back of its head, a strand or two
  // falling forward, a sheen on it.
  if (black) {
    pose.circle(-0.03f, 0.0f, 0.125f, faded(BLACK_HAIR));
    pose.circle(0.03f, -0.085f, 0.05f, faded(BLACK_HAIR));
    pose.circle(0.035f, 0.075f, 0.045f, faded(BLACK_HAIR));
    pose.circle(-0.06f, -0.04f, 0.03f, faded(WithAlpha(Color{ 90, 90, 100, 255 }, 0.8f)));
  }
  // Sunken eyes, glowing red; dead, they go dark. And a gaping mouth.
  for (const float side : { -0.05f, 0.05f }) {
    pose.circle(0.1f, side, 0.033f, faded(Shade(skin, 0.45f)));
    pose.circle(0.105f, side, 0.02f, faded(fallen > 0.0f ? Shade(ZOMBIE_EYES, 0.5f) : ZOMBIE_EYES));
  }
  pose.circle(0.145f, 0.0f, 0.022f, faded(Color{ 60, 20, 20, 255 }));
}

// A number in the middle of `at`, `size` tall, as small as the square needs.
static void Number(int value, Vector2 at, float size, Color color)
{
  const std::string text  = std::to_string(value);
  const int         pixels = std::max(10, int(size));
  DrawText(text.c_str(), int(at.x) - MeasureText(text.c_str(), pixels) / 2, int(at.y) - pixels / 2 + 1, pixels, color);
}

void DrawLives(Vector2 at, float square, int lives)
{
  // A heart in the top right corner: two round lobes and a point.
  const Vector2 c = { at.x + square * 0.3f, at.y - square * 0.3f };
  const float   r = square * 0.13f;
  for (float grow : { 1.25f, 1.0f }) {
    const Color colour = grow > 1.0f ? BADGE_EDGE : HEART;
    const float lobe   = r * grow;
    DrawCircleV({ c.x - r * 0.62f, c.y - r * 0.25f }, lobe * 0.72f, colour);
    DrawCircleV({ c.x + r * 0.62f, c.y - r * 0.25f }, lobe * 0.72f, colour);
    DrawTriangle({ c.x - r * 1.28f * grow, c.y - r * 0.05f }, { c.x, c.y + r * 1.25f * grow },
                 { c.x + r * 1.28f * grow, c.y - r * 0.05f }, colour);
  }
  Number(lives, { c.x, c.y - r * 0.1f }, square * cfg::BADGE_TEXT_SIZE, WHITE);
}

void DrawActions(Vector2 at, float square, int actions)
{
  // A yellow badge in the bottom right corner.
  const Vector2 c = { at.x + square * 0.3f, at.y + square * 0.3f };
  DrawCircleV(c, square * 0.15f, BADGE_EDGE);
  DrawCircleV(c, square * 0.13f, ACTIONS);
  Number(actions, c, square * cfg::BADGE_TEXT_SIZE, BADGE_EDGE);
}

BoardLayout LayOutBoard(int screenWidth, int screenHeight)
{
  // Whole pixels a square, so every line falls on a pixel and they all look
  // the same; the lines thicken a little as the board grows.
  BoardLayout board;
  const int shorter = std::min(screenWidth, screenHeight);
  board.square = std::max(1, int(float(shorter) * (1.0f - 2.0f * cfg::BOARD_MARGIN)) / Goban::SIZE);
  board.line   = std::max(1, board.square / 24);
  board.side   = board.square * Goban::SIZE + board.line;
  board.left   = (screenWidth - board.side) / 2;
  board.top    = (screenHeight - board.side) / 2;
  return board;
}

void DrawBoard(const Goban& goban, int screenWidth, int screenHeight, float fallen)
{
  const auto [left, top, square, line, side] = LayOutBoard(screenWidth, screenHeight);

  // The middle square, under the lines round it.
  constexpr int MIDDLE = Goban::SIZE / 2;
  DrawRectangle(left + MIDDLE * square, top + MIDDLE * square, square + line, square + line, cfg::CENTRE_COLOR);
  for (int i = 0; i <= Goban::SIZE; ++i) {
    DrawRectangle(left + i * square, top, line, side, cfg::GRID_COLOR);
    DrawRectangle(left, top + i * square, side, line, cfg::GRID_COLOR);
  }

  // The soldier and the zombies, each in the middle of its square (inside
  // the lines round it), seen from above; in the square's corner a red heart
  // with its lives, and for the soldier a yellow badge with what is left of
  // its turn.
  const float middle = (float(square) + float(line)) / 2.0f;
  const auto  centre = [&](Position at) {
    return Vector2{ float(left + at.x * square) + middle, float(top + at.y * square) + middle };
  };
  const Position soldier = goban.figureAt();

  for (const Goban::Zombie& zombie : goban.zombies()) {
    DrawZombie(centre(zombie.at), float(square), Facing(zombie.at, soldier), zombie);
    DrawLives(centre(zombie.at), float(square), zombie.lives);
  }

  // The soldier faces the mouse -- up, with it right on top of him.
  const Vector2 at     = centre(soldier);
  const Vector2 mouse  = GetMousePosition();
  const float   dx     = mouse.x - at.x, dy = mouse.y - at.y;
  const float   length = std::sqrt(dx * dx + dy * dy);
  DrawSoldier(at, float(square), length < 1.0f ? Vector2{ 0.0f, -1.0f } : Vector2{ dx / length, dy / length }, fallen);
  DrawLives(at, float(square), goban.figureLives());
  DrawActions(at, float(square), goban.figureActions());
}

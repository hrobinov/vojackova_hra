#include <game/BoardView.hpp>

#include <app/Config.hpp>
#include <game/Figures.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace {

// The badges' colours.
constexpr Color HEART      = Color{ 220, 36, 48, 255 };
constexpr Color BADGE_EDGE = Color{ 40, 20, 20, 255 };
constexpr Color ACTIONS    = Color{ 250, 200, 40, 255 };

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
  figures::DrawSoldier(at, square, facing, fallen);
}

void DrawZombie(Vector2 at, float square, Vector2 facing, const Goban::Zombie& zombie, float fallen, float fade)
{
  figures::DrawZombie(at, square, facing, zombie, fallen, fade);
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

void DrawBoard(const Goban& goban, const Motion& motion, int screenWidth, int screenHeight, float fallen)
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
  const auto  centre = [&](Vector2 at) {
    return Vector2{ float(left) + at.x * float(square) + middle, float(top) + at.y * float(square) + middle };
  };
  // Where the soldier is seen, or on his square before he has been.
  Vector2 soldier = motion.soldier();
  if (soldier.x < -50.0f) soldier = { float(goban.figureAt().x), float(goban.figureAt().y) };

  // Each zombie facing the soldier, leaning off it as it sways.
  for (const Goban::Zombie& zombie : goban.zombies()) {
    const Vector2 seen  = motion.zombie(zombie);
    const float   turn  = std::atan2(soldier.y - seen.y, soldier.x - seen.x) + motion.sway(zombie);
    DrawZombie(centre(seen), float(square), { std::cos(turn), std::sin(turn) }, zombie);
    DrawLives(centre(seen), float(square), zombie.lives);
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

#include <game/BoardView.hpp>

#include <app/Config.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <algorithm>

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

void DrawBoard(const Goban& goban, int screenWidth, int screenHeight)
{
  const auto [left, top, square, line, side] = LayOutBoard(screenWidth, screenHeight);

  DrawRectangle(left, top, side, side, cfg::BOARD_COLOR);
  // The middle square, under the lines round it.
  constexpr int MIDDLE = Goban::SIZE / 2;
  DrawRectangle(left + MIDDLE * square, top + MIDDLE * square, square + line, square + line, cfg::CENTRE_COLOR);
  for (int i = 0; i <= Goban::SIZE; ++i) {
    DrawRectangle(left + i * square, top, line, side, cfg::GRID_COLOR);
    DrawRectangle(left, top + i * square, side, line, cfg::GRID_COLOR);
  }

  // The figure and the zombies, each in the middle of its square (inside the
  // lines round it).
  const float middle = (float(square) + float(line)) / 2.0f;
  const auto  dot    = [&](Position at, Color color) {
    DrawCircleV(Vector2{ float(left + at.x * square) + middle, float(top + at.y * square) + middle },
                float(square) * cfg::DOT_SIZE / 2.0f, color);
  };
  for (const Goban::Zombie& zombie : goban.zombies()) dot(zombie.at, cfg::ZOMBIE_COLOR);
  dot(goban.figureAt(), cfg::DOT_COLOR);
}

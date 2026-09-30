#include <game/BoardView.hpp>

#include <app/Config.hpp>
#include <game/Goban.hpp>

#include <raylib.h>

#include <algorithm>
#include <string>

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
  // lines round it), with small numbers in it: a zombie's lives; the
  // figure's lives, and under a line what is left of its turn.
  const float middle = (float(square) + float(line)) / 2.0f;
  const float radius = float(square) * cfg::DOT_SIZE / 2.0f;
  const auto  centre = [&](Position at) {
    return Vector2{ float(left + at.x * square) + middle, float(top + at.y * square) + middle };
  };
  const auto number = [](int value, int x, int y, int size, Color color) {
    const std::string text = std::to_string(value);
    DrawText(text.c_str(), x - MeasureText(text.c_str(), size) / 2, y, size, color);
  };

  const int zombieText = std::max(8, int(float(square) * cfg::LIVES_TEXT_SIZE));
  for (const Goban::Zombie& zombie : goban.zombies()) {
    const Vector2 at = centre(zombie.at);
    DrawCircleV(at, radius, cfg::ZOMBIE_COLOR);
    number(zombie.lives, int(at.x), int(at.y) - zombieText / 2, zombieText, cfg::ZOMBIE_TEXT_COLOR);
  }

  const Vector2 figure     = centre(goban.figureAt());
  const int     figureText = std::max(6, int(float(square) * cfg::FIGURE_TEXT_SIZE));
  DrawCircleV(figure, radius, cfg::DOT_COLOR);
  number(goban.figureLives(), int(figure.x), int(figure.y) - figureText - 1, figureText, cfg::DOT_TEXT_COLOR);
  DrawLineEx(Vector2{ figure.x - radius * 0.6f, figure.y }, Vector2{ figure.x + radius * 0.6f, figure.y }, 1.0f,
             cfg::DOT_TEXT_COLOR);
  number(goban.figureActions(), int(figure.x), int(figure.y) + 2, figureText, cfg::DOT_TEXT_COLOR);
}

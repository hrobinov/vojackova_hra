// Tuning. Everything worth fiddling with that isn't a style lives here; the
// styles are in ui/Theme.cpp.

#pragma once

#include <raylib.h>

namespace cfg {

// What the window and the task bar call it.
constexpr const char* APP_NAME = "Vojáčková hra";

// Smaller than this and the menu doesn't fit.
constexpr int MIN_WINDOW_W = 640;
constexpr int MIN_WINDOW_H = 480;

// What the window is cleared to, behind the widgets.
constexpr Color BACKGROUND_COLOR = Color{ 24, 24, 28, 255 };

// The board -- the goban: its squares, the lines between them, and the one
// square in the middle, which is always this colour.
constexpr Color BOARD_COLOR = Color{ 222, 184, 135, 255 };
constexpr Color GRID_COLOR  = Color{ 0, 0, 0, 255 };
constexpr Color CENTRE_COLOR = Color{ 200, 30, 30, 255 };
// The figure (the dot the arrow keys move) and the white zombie, and how
// much of its square each fills across.
constexpr Color DOT_COLOR    = Color{ 40, 120, 255, 255 };
constexpr Color ZOMBIE_COLOR = Color{ 255, 255, 255, 255 };
constexpr float DOT_SIZE     = 0.6f;
// How long each of the zombie's actions takes, so they can be followed.
constexpr float ZOMBIE_STEP_SECONDS = 0.3f;
// The gap left round the board, as a share of the window's shorter side.
constexpr float BOARD_MARGIN = 0.05f;

}  // namespace cfg

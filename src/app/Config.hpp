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

// The board -- the goban, grass in a forest (see game/Scenery): the lines
// between its squares, faint but there, and the one square in the middle,
// which is always red.
constexpr Color GRID_COLOR   = Color{ 20, 45, 18, 110 };
constexpr Color CENTRE_COLOR = Color{ 200, 30, 30, 220 };
// How tall the numbers in the hearts and badges are, as a share of a square.
constexpr float BADGE_TEXT_SIZE = 0.26f;
// Where a rocket or the shotgun would hit: tinted this colour, this faint.
constexpr Color AIM_COLOR = Color{ 230, 30, 30, 255 };
constexpr float AIM_ALPHA = 0.28f;
// How long each of the zombie's actions takes, so they can be followed.
constexpr float ZOMBIE_STEP_SECONDS = 0.3f;
// How long the goban stays after the figure dies, before the game's menu.
constexpr float DEATH_SECONDS = 2.0f;
// How long the last of a wave's dead lie there once the next wave has come,
// before they all fade.
constexpr float DEAD_STAY_SECONDS = 1.0f;
// How long the soldier takes to fall, of that.
constexpr float FALL_SECONDS = 0.35f;
// The gap left round the board, as a share of the window's shorter side.
constexpr float BOARD_MARGIN = 0.05f;

}  // namespace cfg

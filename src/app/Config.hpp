// Tuning. Everything worth fiddling with that isn't a style lives here; the
// styles are in ui/Theme.cpp.

#pragma once

#include <raylib.h>

namespace cfg {

// What the window and the task bar call it.
constexpr const char* APP_NAME = "Vojáčková hra";

// The window's size when the game starts.
constexpr int WINDOW_W = 1280;
constexpr int WINDOW_H = 720;

// Smaller than this and the menu doesn't fit.
constexpr int MIN_WINDOW_W = 640;
constexpr int MIN_WINDOW_H = 480;

// What the window is cleared to, behind the widgets.
constexpr Color BACKGROUND_COLOR = Color{ 24, 24, 28, 255 };

}  // namespace cfg

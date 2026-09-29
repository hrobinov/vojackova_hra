// The player's settings, and the config.ini they're kept in between runs.
// Plain data: the settings page edits a copy, and App applies and saves
// whatever changed. Taken over from kovarex_go_editor, less what only a Go
// editor has.

#pragma once

#include <algorithm>
#include <filesystem>

struct Settings {
  struct Graphics {
    bool windowedFullscreen = false;  // a borderless window over the whole monitor
    bool vsync              = true;
    int  fpsLimit           = 0;  // 0 for none; vsync still holds it to the refresh rate

    // How big the whole GUI is drawn. Automatic, as Factorio does it, follows
    // the window's size (AutomaticInterfaceScale), so maximizing the window
    // makes everything bigger. Otherwise it is interfaceScale, in percent,
    // from MIN_SCALE to MAX_SCALE in SCALE_STEP steps. Ctrl and the numpad's
    // + and - step it from anywhere (from the automatic value, which they
    // leave), and Ctrl and numpad 0 go back to automatic.
    bool automaticScale = true;
    int  interfaceScale = 100;

    // The GUI atlas is drawn at half its pixel size, so it stays sharp up to
    // 200%. Below 75% the text stops being readable.
    static constexpr int MIN_SCALE  = 75;
    static constexpr int MAX_SCALE  = 200;
    static constexpr int SCALE_STEP = 25;

    // How long the mouse rests on something before its tooltip shows, in
    // milliseconds, or TOOLTIPS_NEVER. Holding Shift shows them at once
    // whatever this says.
    int tooltipDelay = 200;

    static constexpr int MAX_TOOLTIP_DELAY  = 200;
    static constexpr int TOOLTIP_DELAY_STEP = 10;
    static constexpr int TOOLTIPS_NEVER     = -1;

    bool operator==(const Graphics&) const = default;
  };

  // Not chosen on the settings page but remembered: how the window was left.
  struct Window {
    int  width     = 1280;
    int  height    = 720;
    bool maximized = false;
  };

  Graphics graphics;
  Window   window;

  // A missing file or key keeps the default; out-of-range values are clamped.
  static Settings load();
  void            save() const;

  // %APPDATA%\VojackovaHra\config.ini; config.ini in the working directory
  // where there is no %APPDATA%.
  static std::filesystem::path path();
};

// `percent` as an interface scale the game offers: on the nearest step, and
// within the range.
constexpr int ClampedInterfaceScale(int percent)
{
  using G = Settings::Graphics;
  const int stepped = (percent + G::SCALE_STEP / 2) / G::SCALE_STEP * G::SCALE_STEP;
  return std::clamp(stepped, G::MIN_SCALE, G::MAX_SCALE);
}

// The interface scale for a window `width` x `height` pixels, worked out as
// Factorio's automatic UI scale is: in proportion to a 1920x1080 screen less
// the window's frame, by whichever way it is tighter, rounded down to a step.
// So 100% on full HD. Within the range.
constexpr int AutomaticInterfaceScale(int width, int height)
{
  using G = Settings::Graphics;
  constexpr int FULL_HD_W = 1920 - 64, FULL_HD_H = 1080 - 64;
  const int fit = std::min(width * 100 / FULL_HD_W, height * 100 / FULL_HD_H);
  return std::clamp(fit / G::SCALE_STEP * G::SCALE_STEP, G::MIN_SCALE, G::MAX_SCALE);
}

// The scale the GUI is drawn at, for a window this size.
constexpr int EffectiveInterfaceScale(const Settings::Graphics& graphics, int width, int height)
{
  return graphics.automaticScale ? AutomaticInterfaceScale(width, height) : graphics.interfaceScale;
}

// Brings the window in line with `settings`. Only touches what differs, so
// it's fine to call whenever anything might have changed.
void ApplySettings(const Settings& settings);

// Windowed fullscreen: the window loses its border and covers the monitor it's
// on, but stays an ordinary window -- not topmost, and not exclusive, so the
// monitor never changes mode and alt-tab and other windows behave. (raylib's
// ToggleBorderlessWindowed() makes it topmost, hence doing it by hand.) Going
// back puts the window where it was, maximized if it was.
void SetWindowedFullscreen(bool on);
bool IsWindowedFullscreen();

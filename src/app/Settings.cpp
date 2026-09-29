#include <app/Settings.hpp>

#include <app/Config.hpp>
#include <app/IniFile.hpp>

#include <raylib.h>

#include <algorithm>
#include <cstdlib>

namespace {

// Where the window was before it went windowed fullscreen.
struct Restore {
  bool    active    = false;
  bool    maximized = false;
  Vector2 position  = { 0.0f, 0.0f };
  int     width     = 0;
  int     height    = 0;
} restore;

}  // namespace

Settings Settings::load()
{
  Settings s;
  IniFile ini;
  if (!ini.load(path())) return s;

  s.graphics.windowedFullscreen = ini.getBool("graphics", "windowed-fullscreen", s.graphics.windowedFullscreen);
  s.graphics.vsync              = ini.getBool("graphics", "vsync", s.graphics.vsync);
  s.graphics.fpsLimit           = std::max(0, ini.getInt("graphics", "fps-limit", s.graphics.fpsLimit));
  s.graphics.automaticScale     = ini.getBool("graphics", "automatic-interface-scale", s.graphics.automaticScale);
  s.graphics.interfaceScale     = ClampedInterfaceScale(ini.getInt("graphics", "interface-scale", s.graphics.interfaceScale));
  s.graphics.tooltipDelay       = ini.getInt("graphics", "tooltip-delay", s.graphics.tooltipDelay);
  if (s.graphics.tooltipDelay != Settings::Graphics::TOOLTIPS_NEVER) {
    s.graphics.tooltipDelay = std::clamp(s.graphics.tooltipDelay, 0, Settings::Graphics::MAX_TOOLTIP_DELAY);
  }

  s.window.width     = std::max(cfg::MIN_WINDOW_W, ini.getInt("window", "width", s.window.width));
  s.window.height    = std::max(cfg::MIN_WINDOW_H, ini.getInt("window", "height", s.window.height));
  s.window.maximized = ini.getBool("window", "maximized", s.window.maximized);
  return s;
}

void Settings::save() const
{
  IniFile ini;
  ini.setBool("graphics", "windowed-fullscreen", this->graphics.windowedFullscreen);
  ini.setBool("graphics", "vsync", this->graphics.vsync);
  ini.setInt("graphics", "fps-limit", this->graphics.fpsLimit);
  ini.setBool("graphics", "automatic-interface-scale", this->graphics.automaticScale);
  ini.setInt("graphics", "interface-scale", this->graphics.interfaceScale);
  ini.setInt("graphics", "tooltip-delay", this->graphics.tooltipDelay);

  ini.setInt("window", "width", this->window.width);
  ini.setInt("window", "height", this->window.height);
  ini.setBool("window", "maximized", this->window.maximized);

  if (!ini.save(path(), "Vojackova hra settings. The game rewrites this file, so edit it while the game isn't running.")) {
    TraceLog(LOG_WARNING, "SETTINGS: Couldn't write %s", path().string().c_str());
  }
}

std::filesystem::path Settings::path()
{
  if (const char* appData = std::getenv("APPDATA")) {
    return std::filesystem::path(appData) / "VojackovaHra" / "config.ini";
  }
  return "config.ini";
}

void ApplySettings(const Settings& settings)
{
  if (IsWindowState(FLAG_VSYNC_HINT) != settings.graphics.vsync) {
    if (settings.graphics.vsync) SetWindowState(FLAG_VSYNC_HINT);
    else                         ClearWindowState(FLAG_VSYNC_HINT);
  }
  SetWindowedFullscreen(settings.graphics.windowedFullscreen);
  SetTargetFPS(settings.graphics.fpsLimit);
}

void SetWindowedFullscreen(bool on)
{
  if (on == restore.active) return;

  if (on) {
    restore.maximized = IsWindowMaximized();
    if (restore.maximized) RestoreWindow();  // so the size saved is the one to go back to
    restore.position = GetWindowPosition();
    restore.width    = GetScreenWidth();
    restore.height   = GetScreenHeight();

    const int     monitor = GetCurrentMonitor();
    const Vector2 origin  = GetMonitorPosition(monitor);
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowPosition(int(origin.x), int(origin.y));
    // One pixel taller than the monitor. A GL window that covers it exactly
    // gets promoted by the driver to what is effectively exclusive fullscreen
    // -- a black flash on alt-tab, nothing able to draw over it -- which is
    // what this mode is here to avoid. The spare row is off the bottom edge.
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor) + 1);
  } else {
    ClearWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowSize(restore.width, restore.height);
    SetWindowPosition(int(restore.position.x), int(restore.position.y));
    if (restore.maximized) MaximizeWindow();
  }
  restore.active = on;
}

bool IsWindowedFullscreen()
{
  return restore.active;
}

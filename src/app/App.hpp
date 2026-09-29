// The application: the window, the settings and the frame loop. Each frame
// the keyboard shortcuts are read first, then the Gui runs, then whatever the
// menu asked for is done, then everything is drawn.

#pragma once

#include <app/Settings.hpp>
#include <ui/GuiLayer.hpp>

class App {
public:
  App();

  // Runs until the window closes.
  int run();

private:
  // Opens the window on construction and closes it on destruction. Declared
  // before every member that needs the GL context, so it brackets them.
  struct Window {
    explicit Window(const Settings& settings);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
  };

  // The interface scale's shortcuts: a step up or down, or back to automatic.
  enum class Scale { Up, Down, Automatic };

  void frame();
  void handleKeys();
  void handleMenu();
  void updateSettings();
  void scale(Scale how);

  // Read before the window opens, which is sized from it.
  Settings settings = Settings::load();
  Settings applied  = this->settings;  // what the window was last set to
  int      shownScale = 0;             // the interface scale the GUI is drawn at

  Window       window{ this->settings };
  ui::GuiLayer gui{ this->settings };

  bool quitRequested = false;
};

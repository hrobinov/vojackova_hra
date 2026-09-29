// The application: the window and the frame loop. Each frame the Gui runs,
// then whatever the menu asked for is done, then everything is drawn.

#pragma once

#include <ui/GuiLayer.hpp>

class App {
public:
  App() = default;

  // Runs until the window closes.
  int run();

private:
  // Opens the window on construction and closes it on destruction. Declared
  // before every member that needs the GL context, so it brackets them.
  struct Window {
    Window();
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
  };

  void frame();
  void handleMenu();

  Window       window;
  ui::GuiLayer gui;

  bool quitRequested = false;
};

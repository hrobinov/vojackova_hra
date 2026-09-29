// Vojáčková hra -- a game Robin is making with the help of AI.
//
// So far it is only the main menu:
//
//   app/     the window and the main loop
//   ui/      everything on screen: the main menu, its pages, and the Agui
//            backend and Factorio-like theme they are built from (taken over
//            from https://github.com/kovarex/kovarex_go_editor)

#include <app/App.hpp>

int main()
{
  App app;
  return app.run();
}

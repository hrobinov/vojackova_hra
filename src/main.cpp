// Vojáčková hra -- a game Robin is making with the help of AI.
//
// So far: the main menu, games to name, save, load and delete, and on the
// goban the figure against waves of white zombies.
//
//   app/     the window and the main loop
//   game/    a game, its save, and the round on the goban and how it's drawn
//   ui/      everything on screen: the main menu, its pages, and the Agui
//            backend and Factorio-like theme they are built from (taken over
//            from https://github.com/kovarex/kovarex_go_editor)

#include <app/App.hpp>

int main()
{
  App app;
  return app.run();
}

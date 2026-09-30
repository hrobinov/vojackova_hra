// What the goban lies in: a clearing in a forest, seen from above but in the
// round, lit by the game's one light (relief::LightDirection).
//
// The ground rises and falls a little -- grass on the clearing, each square
// its own green, with blades, flowers and darker patches; the forest floor
// round it, darker, strewn with fallen leaves -- and is lit as it slopes.
// Round the clearing: bushes and mossy stones at its edge, and beyond them
// trees -- broad-leaved crowns of clumped leaves, and pines, tier upon tier
// -- each built of solids (see Relief), lit, and casting a soft shadow,
// some of it onto the clearing. Only a picture: nothing grows on the
// squares, and everywhere can be walked on.
//
// Painted once into a texture the size of the window, and again only when
// the window changes size, so it costs nothing a frame. The trees, bushes
// and stones are built once, the first time.

#pragma once

#include <raylib.h>

#include <vector>

class Scenery {
public:
  Scenery() = default;
  ~Scenery();
  Scenery(const Scenery&) = delete;
  Scenery& operator=(const Scenery&) = delete;

  // Paints the scenery for a screen this size, unless it is already painted.
  // Not between a push and a pop of the matrix: it draws to its own texture.
  void prepare(int screenWidth, int screenHeight);

  // The scenery, filling the screen.
  void draw() const;

private:
  // A tree, bush or stone: lit, and its shadow.
  struct Thing {
    Texture2D lit{}, shadow{};
  };

  void build();
  void paint(int screenWidth, int screenHeight);

  RenderTexture2D    canvas{};
  bool               painted = false;
  std::vector<Thing> leafy, pines, bushes, stones;
};

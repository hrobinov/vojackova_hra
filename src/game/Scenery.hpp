// What the goban lies in: a clearing in a forest. The squares are grass --
// each a slightly different green, with blades, the odd flower and darker
// patches -- and round the goban, on the forest floor, trees seen from
// above: broad-leaved and pines, each with its shadow. Only a picture:
// nothing grows on the squares, and everywhere can be walked on.
//
// Painted once into a texture the size of the window, and again only when
// the window changes size, so it costs nothing a frame.

#pragma once

#include <raylib.h>

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
  void paint(int screenWidth, int screenHeight);

  RenderTexture2D canvas{};
  bool            painted = false;
};

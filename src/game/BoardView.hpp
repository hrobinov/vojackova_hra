// Draws the goban: a square of Goban::SIZE x Goban::SIZE squares in the
// middle of the window, as big as fits with a margin round it -- its lines,
// over the grass Scenery paints -- the square in its middle red, and the
// soldier (the figure) and the zombies on their squares, seen from above,
// each with its lives in a heart; the soldier, facing the mouse, with what
// is left of its turn too.

#pragma once

#include <game/Goban.hpp>

#include <raylib.h>

// Where the board goes on a screen `screenWidth` x `screenHeight`: its top
// left corner, how big a square is and how thick a line, and how big the
// whole board is, lines and all.
struct BoardLayout {
  int left   = 0;
  int top    = 0;
  int square = 0;
  int line   = 0;
  int side   = 0;
};
BoardLayout LayOutBoard(int screenWidth, int screenHeight);

// `fallen`: how far the soldier, dead, has fallen, from 0 (not at all) to 1.
void DrawBoard(const Goban& goban, int screenWidth, int screenHeight, float fallen = 0.0f);

// The way someone at `from` faces to look at `to`, a unit long; up if they
// are on the same square.
Vector2 Facing(Position from, Position to);

// The soldier and a zombie, seen from above, in the middle of a square
// `square` pixels across, facing `facing`. A white zombie in rags, which
// where it is makes a little different from the others; a black one with
// black hair and a black shirt.
// A dead soldier falls: over onto his side, and his helmet off, `fallen`
// from 0 to 1.
void DrawSoldier(Vector2 at, float square, Vector2 facing, float fallen = 0.0f);
// A dead zombie falls, `fallen` from 0 to 1, and its body fades, `fade`
// from 1 (all there) to 0 (gone).
void DrawZombie(Vector2 at, float square, Vector2 facing, const Goban::Zombie& zombie, float fallen = 0.0f,
                float fade = 1.0f);

// In the corner of the square: a heart with someone's lives, and a badge with
// what is left of the soldier's turn.
void DrawLives(Vector2 at, float square, int lives);
void DrawActions(Vector2 at, float square, int actions);

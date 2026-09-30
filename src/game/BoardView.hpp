// Draws the goban: a square of Goban::SIZE x Goban::SIZE squares in the
// middle of the window, as big as fits with a margin round it, the square in
// its middle red, and the figure and the white zombies on their squares,
// each with its lives in it -- the figure with what is left of its turn too.

#pragma once

class Goban;

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

void DrawBoard(const Goban& goban, int screenWidth, int screenHeight);

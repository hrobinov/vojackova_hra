// A round on the goban: the figure (the blue dot) against white zombies,
// wave after wave. Every Start begins a new one, and nothing of it is saved.
//
// It is played in turns. First the figure: each of its actions is a step, a
// square left, right, up or down, onto a free square of the goban; a shot of
// its pistol at a zombie up to PISTOL_RANGE squares away in a straight line,
// taking as many of its lives as the figure wounds; or waiting, doing
// nothing. Then the zombies, all of them an action at a time: each action a
// step towards the figure, or once beside it, an attack that takes as many of
// the figure's lives as a zombie wounds. Then the figure again.
//
// A zombie with no lives left is dead and gone. When the last of a wave is,
// the figure is back in the middle with a whole turn, and the next wave
// comes, one zombie more: the first where the first always starts, the
// others anywhere free but the middle, somewhere else every time.

#pragma once

#include <random>
#include <vector>

// A square of the goban, counted from the top left.
struct Position {
  int x = 0;
  int y = 0;

  bool operator==(const Position&) const = default;
};

// Someone's basic properties, as the game's menu shows them.
struct Properties {
  int actions = 0;
  int lives   = 0;
  int wounds  = 0;
};

class Goban {
public:
  // SIZE x SIZE squares.
  static constexpr int SIZE = 19;

  // How far the figure's pistol reaches, in squares, left, right, up or down
  // -- not across a corner. It never runs out.
  static constexpr int PISTOL_RANGE = 3;

  // Where the figure starts, in the middle, and the first zombie of every
  // wave, below it at the bottom edge.
  static constexpr Position FIGURE_START{ SIZE / 2, SIZE / 2 };
  static constexpr Position ZOMBIE_START{ SIZE / 2, SIZE - 1 };

  struct Zombie {
    Position at;
    int      lives = 0;
  };

  // A new round, for a figure and zombies with these properties: the figure
  // where it starts with all its lives, and to move, and the first wave -- a
  // single zombie.
  void start(const Properties& figure, const Properties& zombie);

  // The figure's step a square that way, if it is its turn and the square is
  // on the goban and free. A step not taken costs no action.
  void move(int dx, int dy);

  // The figure shoots at the square `at`, if it is its turn and a zombie is
  // there, in range. A shot not taken costs no action.
  void shoot(Position at);

  // The figure waits, if it is its turn: an action spent on nothing.
  void wait();

  // One action of every zombie, if it is their turn, one zombie after
  // another. Beside the figure (not across a corner), an attack; otherwise a
  // square towards the figure, along whichever way it is further from it --
  // or the other way, if another zombie is in the way.
  void zombieAction();

  bool zombiesTurn() const { return this->zombieActionsLeft > 0; }

  Position                   figureAt() const { return this->figure; }
  int                        figureLives() const { return this->lives; }
  const std::vector<Zombie>& zombies() const { return this->horde; }

private:
  // One of the figure's actions used; the last hands the turn to the zombies.
  void spendFigureAction();
  // The next wave, of `count` zombies.
  void spawn(int count);
  bool free(Position at) const;

  Properties figureProperties;
  Properties zombieProperties;

  Position            figure = FIGURE_START;
  int                 lives  = 0;
  std::vector<Zombie> horde;
  int                 wave = 0;  // how many zombies the last wave had

  // What is left of the turn: the figure's actions, and once they are all
  // used, the zombies'. The zombies are on the move while they have any.
  int figureActionsLeft = 0;
  int zombieActionsLeft = 0;

  std::mt19937 random{ std::random_device{}() };
};

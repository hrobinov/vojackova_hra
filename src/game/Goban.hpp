// A round on the goban: the figure -- the soldier -- against zombies, wave
// after wave. Every Start begins a new one, and nothing of it is saved.
//
// It is played in turns. First the figure. Each of its actions is one of:
//   - a step, a square left, right, up or down, onto a free square;
//   - a shot of its pistol at a zombie as many squares away as its range in
//     a straight line, or anywhere NEAR squares round it, across corners
//     too, taking as many of its lives as the figure wounds;
//   - a rocket, if it has one left this round, at any square, taking
//     ROCKET_WOUNDS lives from everyone on it and on the eight squares round
//     it -- the figure too, if it is that close;
//   - a shotgun's shell, if it has one left this round, at a square right
//     beside it, taking SHOTGUN_WOUNDS lives from every zombie on that square
//     and the two either side of it round the figure;
//   - waiting, doing nothing.
// Then the zombies, all of them an action at a time, each as many as it has
// -- a black zombie one more than a white one. Each action is a step towards
// the figure, or once beside it, an attack that takes as many of the
// figure's lives as the zombie wounds. Then the figure again -- unless it
// has no lives left, and is dead: then nobody moves any more.
//
// A zombie with no lives left is dead and gone, and earns the figure
// ZOMBIE_REWARD, or a black one BLACK_ZOMBIE_REWARD. When the last of a wave is, the figure is back in the middle
// with a whole turn, and the next wave comes, MORE_A_WAVE zombies more: the first
// where the first always starts, the others anywhere free but the middle,
// somewhere else every time.

#pragma once

#include <array>
#include <optional>
#include <random>
#include <utility>
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
  int range   = 0;  // how far its pistol reaches, in squares; none without one
  int rockets = 0;  // how many rockets it has, each round afresh
  int shells  = 0;  // how many shotgun shells it has, each round afresh
};

class Goban {
public:
  // SIZE x SIZE squares.
  static constexpr int SIZE = 19;

  // What a dead zombie earns, in Kč: a white one, a black one, a red one.
  static constexpr int ZOMBIE_REWARD       = 1;
  static constexpr int BLACK_ZOMBIE_REWARD = 3;
  static constexpr int RED_ZOMBIE_REWARD   = 5;

  // What a rocket and a shotgun's shell take from every zombie they hit.
  static constexpr int ROCKET_WOUNDS  = 3;
  static constexpr int SHOTGUN_WOUNDS = 3;

  // Where the figure starts, in the middle, and the first zombie of every
  // wave, below it at the bottom edge.
  static constexpr Position FIGURE_START{ SIZE / 2, SIZE / 2 };
  static constexpr Position ZOMBIE_START{ SIZE / 2, SIZE - 1 };

  // How far round it, in every direction, the figure's pistol reaches, the
  // square of it across corners too -- further only in a straight line.
  static constexpr int NEAR = 2;

  // How many zombies more each wave has than the one before: MORE_A_WAVE,
  // and from wave EVEN_MORE_FROM on, EVEN_MORE_A_WAVE.
  static constexpr int MORE_A_WAVE      = 2;
  static constexpr int EVEN_MORE_FROM   = 10;
  static constexpr int EVEN_MORE_A_WAVE = 3;

  // The kinds of zombie. Each zombie of a wave may be black rather than
  // white: from BLACK_FROM on with a BLACK_CHANCE in 100, from MORE_BLACK_FROM
  // on with a MORE_BLACK_CHANCE in 100. And one that would be white, from
  // RED_FROM on, may be red instead, with a RED_CHANCE in 100.
  enum class Kind { White, Black, Red };
  static constexpr int BLACK_FROM        = 5;
  static constexpr int BLACK_CHANCE      = 10;
  static constexpr int MORE_BLACK_FROM   = 10;
  static constexpr int MORE_BLACK_CHANCE = 25;
  static constexpr int RED_FROM          = 15;
  static constexpr int RED_CHANCE        = 25;

  // How many zombies wave `wave` has.
  static int WaveSize(int wave);

  struct Zombie {
    Position at;
    int      lives = 0;
    Kind     kind  = Kind::White;
    unsigned look  = 0;  // which of the ways a zombie can look, picked as it comes on
    unsigned id    = 0;  // which zombie it is, none other the same, to follow it by
  };

  // A new round, for a figure and zombies of each kind with these
  // properties: the figure where it starts with all its lives, and to move,
  // and the wave `wave` -- the first, a single zombie, unless it starts
  // further on.
  void start(const Properties& figure, const Properties& white, const Properties& black, const Properties& red,
             int wave = 1);

  // The figure's step a square that way, if it is its turn and the square is
  // on the goban and free. A step not taken costs no action.
  void move(int dx, int dy);

  // The figure shoots at the square `at`, if it is its turn and a zombie is
  // there, in range. A shot not taken costs no action. False if it didn't
  // shoot.
  bool shoot(Position at);

  // The figure fires a rocket at the square `at`, if it is its turn and it
  // has one: every zombie on it or round it, across the corners too, is hit.
  // False if it didn't fire.
  bool fireRocket(Position at);

  // The figure fires the shotgun at `at`, one of the eight
  // squares round it, if it is its turn and it has a shell: every zombie on
  // it, or on the square either side of it round the figure, is hit. False
  // if it didn't fire.
  bool fireShotgun(Position at);

  // The squares a shotgun at `figure` fired at `at` hits: `at`, and either
  // side of it round the figure -- so up hits up and the two corners by it,
  // and a corner hits the corner and the two squares by it. Some may be off
  // the goban. Nothing if `at` isn't right beside the figure.
  static std::optional<std::array<Position, 3>> ShotgunSpread(Position figure, Position at);

  // The figure waits, if it is its turn: an action spent on nothing.
  void wait();

  // One action of every zombie that has one left this turn, if it is their
  // turn, one zombie after another. Beside the figure (not across a corner), an attack; otherwise a
  // square towards the figure, along whichever way it is further from it --
  // or the other way, if another zombie is in the way.
  void zombieAction();

  bool zombiesTurn() const { return this->zombieActionsLeft > 0; }

  Position                   figureAt() const { return this->figure; }
  int                        figureLives() const { return this->lives; }
  // What the figure has left of its turn: none while the zombies move.
  int                        figureActions() const { return this->figureActionsLeft; }
  int                        rocketsLeft() const { return this->rockets; }
  int                        shellsLeft() const { return this->shells; }
  // What the figure started the round with.
  const Properties&          figureStart() const { return this->figureProperties; }
  const std::vector<Zombie>& zombies() const { return this->horde; }
  // Which wave it is -- the round, as the goban's title calls it: 1 to start
  // with, and one more with each wave, which has MORE_A_WAVE zombies more
  // than the one before -- the first, a single one.
  int                        waveNumber() const { return this->wave; }

  // The zombies killed since this was last asked, and none left to ask about.
  std::vector<Zombie> takeDeaths() { return std::exchange(this->deaths, {}); }

  // What the zombies killed this round have earned, in Kč.
  int earnings() const { return this->earned; }
  // The earnings, for the game's money, leaving none -- so they can't be
  // taken twice.
  int takeEarnings() { return std::exchange(this->earned, 0); }

private:
  // One of the figure's actions used; the last hands the turn to the zombies.
  void spendFigureAction();
  // After the figure has hit zombies: the dead ones gone and earned, and the
  // action spent -- or with the last of the wave dead, the next wave.
  void afterHit();
  // Wave `wave`, of as many zombies as it has.
  void spawn(int wave);
  bool free(Position at) const;

  const Properties& properties(Kind kind) const { return this->zombieProperties[int(kind)]; }

  Properties figureProperties;
  Properties zombieProperties[3];  // white, black, red

  Position            figure = FIGURE_START;
  int                 lives  = 0;
  int                 rockets = 0;
  int                 shells  = 0;
  std::vector<Zombie> horde;
  int                 wave = 0;  // how many zombies the last wave had
  int                 earned = 0;
  std::vector<Zombie> deaths;
  unsigned            lastId = 0;

  // What is left of the turn: the figure's actions, and once they are all
  // used, the zombies'. The zombies are on the move while they have any.
  int figureActionsLeft = 0;
  int zombieActionsLeft = 0;
  int zombieActions     = 0;  // how many the zombies' turn has in all

  std::mt19937 random{ std::random_device{}() };
};

#include <game/Goban.hpp>

#include <algorithm>
#include <cstdlib>

namespace {

bool OnGoban(Position at)
{
  return at.x >= 0 && at.x < Goban::SIZE && at.y >= 0 && at.y < Goban::SIZE;
}

int Sign(int value)
{
  return (value > 0) - (value < 0);
}

}  // namespace

void Goban::start(const Properties& figureStarts, const Properties& zombieStarts)
{
  this->figureProperties  = figureStarts;
  this->zombieProperties  = zombieStarts;
  this->figure            = FIGURE_START;
  this->lives             = figureStarts.lives;
  this->figureActionsLeft = figureStarts.actions;
  this->zombieActionsLeft = 0;
  this->spawn(1);
}

void Goban::move(int dx, int dy)
{
  if (this->figureActionsLeft == 0) return;
  const Position to{ this->figure.x + dx, this->figure.y + dy };
  if (!OnGoban(to) || !this->free(to)) return;
  this->figure = to;
  this->spendFigureAction();
}

void Goban::shoot(Position at)
{
  if (this->figureActionsLeft == 0) return;
  const auto target = std::find_if(this->horde.begin(), this->horde.end(), [at](const Zombie& z) { return z.at == at; });
  if (target == this->horde.end()) return;
  // In a straight line, and no further than the pistol reaches.
  const int dx = std::abs(at.x - this->figure.x);
  const int dy = std::abs(at.y - this->figure.y);
  if ((dx != 0 && dy != 0) || dx + dy > PISTOL_RANGE) return;

  target->lives -= this->figureProperties.wounds;
  if (target->lives <= 0) this->horde.erase(target);
  // The last of the wave: the figure back in the middle for the next, and
  // with the whole of a turn before it.
  if (this->horde.empty()) {
    this->figure = FIGURE_START;
    this->spawn(this->wave + 1);
    this->figureActionsLeft = this->figureProperties.actions;
    return;
  }
  this->spendFigureAction();
}

void Goban::wait()
{
  if (this->figureActionsLeft > 0) this->spendFigureAction();
}

void Goban::spendFigureAction()
{
  if (--this->figureActionsLeft == 0) this->zombieActionsLeft = this->zombieProperties.actions;
}

void Goban::zombieAction()
{
  if (this->zombieActionsLeft == 0) return;

  for (Zombie& zombie : this->horde) {
    const int dx = this->figure.x - zombie.at.x;
    const int dy = this->figure.y - zombie.at.y;
    if (std::abs(dx) + std::abs(dy) == 1) {
      // Beside the figure: an attack. Lives don't go below none.
      this->lives = std::max(0, this->lives - this->zombieProperties.wounds);
      continue;
    }
    // A step towards the figure, along the way it is further off -- up or
    // down when it is as far both ways -- or the other way towards it when
    // another zombie is in the way. Neither way is onto the figure: that
    // would take being beside it.
    const Position across{ zombie.at.x + Sign(dx), zombie.at.y };
    const Position along{ zombie.at.x, zombie.at.y + Sign(dy) };
    const bool     acrossFirst = std::abs(dx) > std::abs(dy);
    for (const Position& to : { acrossFirst ? across : along, acrossFirst ? along : across }) {
      if (to != zombie.at && this->free(to)) {
        zombie.at = to;
        break;
      }
    }
  }

  if (--this->zombieActionsLeft == 0) this->figureActionsLeft = this->figureProperties.actions;
}

void Goban::spawn(int count)
{
  this->wave = count;
  this->horde.clear();
  // The first where it always starts, unless the figure is standing there.
  if (this->free(ZOMBIE_START)) this->horde.push_back({ ZOMBIE_START, this->zombieProperties.lives });
  // The rest on free squares picked at random -- never the red one in the
  // middle, where the figure starts. There are always enough, but for a wave
  // bigger than the goban.
  while (int(this->horde.size()) < count) {
    std::vector<Position> squares;
    for (int y = 0; y < SIZE; ++y) {
      for (int x = 0; x < SIZE; ++x) {
        if (Position{ x, y } != FIGURE_START && this->free({ x, y })) squares.push_back({ x, y });
      }
    }
    if (squares.empty()) break;
    std::uniform_int_distribution<size_t> pick(0, squares.size() - 1);
    this->horde.push_back({ squares[pick(this->random)], this->zombieProperties.lives });
  }
}

bool Goban::free(Position at) const
{
  if (at == this->figure) return false;
  return std::none_of(this->horde.begin(), this->horde.end(), [at](const Zombie& z) { return z.at == at; });
}

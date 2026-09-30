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

void Goban::start(const Properties& figureStarts, const Properties& white, const Properties& black, int firstWave)
{
  this->figureProperties    = figureStarts;
  this->zombieProperties[0] = white;
  this->zombieProperties[1] = black;
  this->figure            = FIGURE_START;
  this->lives             = figureStarts.lives;
  this->rockets           = figureStarts.rockets;
  this->shells            = figureStarts.shells;
  this->figureActionsLeft = figureStarts.actions;
  this->zombieActionsLeft = 0;
  this->earned            = 0;
  this->deaths.clear();
  this->spawn(firstWave);
}

void Goban::move(int dx, int dy)
{
  if (this->figureActionsLeft == 0) return;
  const Position to{ this->figure.x + dx, this->figure.y + dy };
  if (!OnGoban(to) || !this->free(to)) return;
  this->figure = to;
  this->spendFigureAction();
}

bool Goban::shoot(Position at)
{
  if (this->figureActionsLeft == 0) return false;
  const auto target = std::find_if(this->horde.begin(), this->horde.end(), [at](const Zombie& z) { return z.at == at; });
  if (target == this->horde.end()) return false;
  // In a straight line, no further than the pistol reaches -- or across a
  // corner, but only right beside the figure.
  const int  dx       = std::abs(at.x - this->figure.x);
  const int  dy       = std::abs(at.y - this->figure.y);
  const bool straight = (dx == 0 || dy == 0) && dx + dy <= this->figureProperties.range;
  const bool corner   = dx == 1 && dy == 1;
  if (!straight && !corner) return false;

  target->lives -= this->figureProperties.wounds;
  this->afterHit();
  return true;
}

bool Goban::fireRocket(Position at)
{
  if (this->figureActionsLeft == 0 || this->rockets == 0 || !OnGoban(at)) return false;
  --this->rockets;
  const auto inBlast = [at](Position square) { return std::abs(square.x - at.x) <= 1 && std::abs(square.y - at.y) <= 1; };
  for (Zombie& zombie : this->horde) {
    if (inBlast(zombie.at)) zombie.lives -= ROCKET_WOUNDS;
  }
  // Too close, and the figure is hit too.
  if (inBlast(this->figure)) this->lives = std::max(0, this->lives - ROCKET_WOUNDS);
  this->afterHit();
  return true;
}

std::optional<std::array<Position, 3>> Goban::ShotgunSpread(Position figure, Position at)
{
  // The eight squares round the figure, going round.
  constexpr Position RING[] = { { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 } };
  const Position offset{ at.x - figure.x, at.y - figure.y };
  for (int i = 0; i < 8; ++i) {
    if (RING[i] != offset) continue;
    const auto square = [&](int j) {
      const Position& o = RING[(j + 8) % 8];
      return Position{ figure.x + o.x, figure.y + o.y };
    };
    return std::array<Position, 3>{ square(i - 1), square(i), square(i + 1) };
  }
  return std::nullopt;
}

bool Goban::fireShotgun(Position at)
{
  if (this->figureActionsLeft == 0 || this->shells == 0) return false;
  const std::optional<std::array<Position, 3>> spread = ShotgunSpread(this->figure, at);
  if (!spread) return false;
  --this->shells;
  for (Zombie& zombie : this->horde) {
    if (std::find(spread->begin(), spread->end(), zombie.at) != spread->end()) zombie.lives -= SHOTGUN_WOUNDS;
  }
  this->afterHit();
  return true;
}

void Goban::afterHit()
{
  for (const Zombie& zombie : this->horde) {
    if (zombie.lives > 0) continue;
    this->deaths.push_back(zombie);
    this->earned += zombie.kind == Kind::Black ? BLACK_ZOMBIE_REWARD : ZOMBIE_REWARD;
  }
  std::erase_if(this->horde, [](const Zombie& zombie) { return zombie.lives <= 0; });
  // The figure killed by its own rocket: nobody moves any more.
  if (this->lives == 0) {
    this->figureActionsLeft = 0;
    this->zombieActionsLeft = 0;
    return;
  }
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
  if (--this->figureActionsLeft > 0) return;
  // The zombies' turn: as many actions as the busiest of them has.
  this->zombieActions = 0;
  for (const Zombie& zombie : this->horde) this->zombieActions = std::max(this->zombieActions, this->properties(zombie.kind).actions);
  this->zombieActionsLeft = this->zombieActions;
  if (this->zombieActionsLeft == 0) this->figureActionsLeft = this->figureProperties.actions;
}

void Goban::zombieAction()
{
  if (this->zombieActionsLeft == 0) return;

  // Which of the turn's actions this is: a zombie with fewer sits it out.
  const int action = this->zombieActions - this->zombieActionsLeft;
  for (Zombie& zombie : this->horde) {
    if (action >= this->properties(zombie.kind).actions) continue;
    const int dx = this->figure.x - zombie.at.x;
    const int dy = this->figure.y - zombie.at.y;
    if (std::abs(dx) + std::abs(dy) == 1) {
      // Beside the figure: an attack. Lives don't go below none.
      this->lives = std::max(0, this->lives - this->properties(zombie.kind).wounds);
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

  // A dead figure: nobody moves any more.
  if (this->lives == 0) {
    this->zombieActionsLeft = 0;
    return;
  }
  if (--this->zombieActionsLeft == 0) this->figureActionsLeft = this->figureProperties.actions;
}

void Goban::spawn(int count)
{
  this->wave = count;
  this->horde.clear();
  // Each zombie black by chance, the further the wave the likelier.
  const int chance = count >= MORE_BLACK_FROM ? MORE_BLACK_CHANCE : count >= BLACK_FROM ? BLACK_CHANCE : 0;
  std::uniform_int_distribution<int> percent(0, 99);
  const auto kind = [&] { return percent(this->random) < chance ? Kind::Black : Kind::White; };
  // The first where it always starts, unless the figure is standing there.
  if (this->free(ZOMBIE_START)) {
    const Kind first = kind();
    this->horde.push_back({ ZOMBIE_START, this->properties(first).lives, first });
  }
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
    const Kind which = kind();
    this->horde.push_back({ squares[pick(this->random)], this->properties(which).lives, which });
  }
}

bool Goban::free(Position at) const
{
  if (at == this->figure) return false;
  return std::none_of(this->horde.begin(), this->horde.end(), [at](const Zombie& z) { return z.at == at; });
}

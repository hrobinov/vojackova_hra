#include <game/Scenery.hpp>

#include <game/BoardView.hpp>
#include <game/Goban.hpp>
#include <game/Relief.hpp>

#include <algorithm>
#include <cmath>
#include <random>

namespace {

using relief::Ball;
using relief::Ellipsoid;
using relief::Material;
using relief::Solid;

// The same forest every time.
constexpr unsigned SEED = 20260930;

// How many of each kind of thing are built, and at how many pixels across.
constexpr int LEAFY  = 8;
constexpr int PINES  = 4;
constexpr int BUSHES = 4;
constexpr int STONES = 4;
constexpr int PICTURE = 192;

// The ground is worked out at this fraction of the screen's pixels, and
// smoothed up to it.
constexpr int GROUND_EVERY = 2;

// The ground's light: how bright where it faces away, and at most.
constexpr float AMBIENT = 0.55f;
constexpr float DIFFUSE = 0.6f;

// The ground's colours.
constexpr Color GRASS_DARK   = Color{ 74, 128, 54, 255 };
constexpr Color GRASS_LIGHT  = Color{ 110, 162, 70, 255 };
constexpr Color FLOOR_DARK   = Color{ 38, 52, 30, 255 };
constexpr Color FLOOR_LIGHT  = Color{ 66, 78, 42, 255 };
constexpr Color LEAF_LITTER[] = { { 120, 84, 44, 255 }, { 150, 104, 50, 255 }, { 100, 70, 40, 255 }, { 80, 90, 44, 255 } };
constexpr Color BLADE_DARK   = Color{ 56, 100, 40, 255 };
constexpr Color BLADE_LIGHT  = Color{ 150, 196, 96, 255 };
constexpr Color FLOWERS[]    = { { 245, 240, 230, 255 }, { 250, 214, 70, 255 }, { 230, 120, 170, 255 }, { 150, 170, 250, 255 } };

// Leaves, needles, bark and stone.
constexpr Color LEAVES[]  = { { 30, 70, 34, 255 }, { 44, 96, 44, 255 }, { 64, 124, 56, 255 }, { 92, 150, 70, 255 } };
constexpr Color NEEDLES[] = { { 20, 52, 38, 255 }, { 28, 70, 48, 255 }, { 40, 90, 60, 255 } };
constexpr Color BERRY     = Color{ 190, 30, 40, 255 };
constexpr Color STONE     = Color{ 128, 128, 122, 255 };
constexpr Color MOSS      = Color{ 86, 120, 50, 255 };

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

Color Mix(Color a, Color b, float t)
{
  t = std::clamp(t, 0.0f, 1.0f);
  return Color{ Byte(a.r + (b.r - a.r) * t), Byte(a.g + (b.g - a.g) * t), Byte(a.b + (b.b - a.b) * t), 255 };
}

Color Scaled(Color c, float by)
{
  return Color{ Byte(c.r * by), Byte(c.g * by), Byte(c.b * by), c.a };
}

// Noise in several layers, finer and fainter each: the lie of the ground.
float Rolling(float x, float y, unsigned seed)
{
  return 0.55f * relief::Noise(x, y, seed) + 0.3f * relief::Noise(x * 2.3f, y * 2.3f, seed + 1)
       + 0.15f * relief::Noise(x * 5.1f, y * 5.1f, seed + 2);
}

// ---------------------------------------------------------------- the things

// A broad-leaved crown: a dark mass, clumps of leaves heaped over it,
// lighter the higher they are.
std::vector<Solid> Leafy(std::mt19937& random)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  std::vector<Solid> s;
  // Each tree its own green: some warmer, some bluer, some darker.
  const float red = 0.85f + 0.35f * unit(random), blue = 0.8f + 0.4f * unit(random), dark = 0.85f + 0.25f * unit(random);
  const auto  leaf = [&](Color c) {
    return Color{ Byte(c.r * red * dark), Byte(c.g * dark), Byte(c.b * blue * dark), 255 };
  };
  s.push_back(Ellipsoid({ 0, 0, 0.25f }, { 0.78f, 0.78f, 0.45f }, { leaf(LEAVES[0]), 0.1f, 0.15f }));
  for (int i = 0; i < 34; ++i) {
    const float angle = 2.0f * PI * unit(random), out = 0.72f * std::sqrt(unit(random));
    const float z     = 0.4f + 0.38f * (1.0f - out / 0.72f);
    const int   tone  = std::min(3, int(z * 3.2f + unit(random) * 0.8f));
    s.push_back(Ball({ std::cos(angle) * out, std::sin(angle) * out, z }, 0.14f + 0.12f * unit(random),
                     { leaf(LEAVES[tone]), 0.15f, 0.18f }));
  }
  return s;
}

// A pine: tier upon tier of needled boughs, each smaller and higher, and
// the tip on top.
std::vector<Solid> Pine(std::mt19937& random)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  std::vector<Solid> s;
  constexpr int TIERS = 5;
  for (int t = 0; t < TIERS; ++t) {
    const float reach = 0.86f * (1.0f - float(t) / float(TIERS));
    const float z     = 0.12f + 0.16f * float(t);
    const int   boughs = 11 - t * 2;
    const float turn  = unit(random) * 2.0f * PI;
    for (int b = 0; b < boughs; ++b) {
      const float angle = turn + 2.0f * PI * float(b) / float(boughs);
      const Color c     = NEEDLES[std::min(2, t / 2 + int(unit(random) * 1.5f))];
      s.push_back(Ellipsoid({ std::cos(angle) * reach * 0.62f, std::sin(angle) * reach * 0.62f, z },
                            { reach * 0.42f, reach * 0.42f, 0.14f }, { c, 0.2f, 0.2f }));
    }
    s.push_back(Ellipsoid({ 0, 0, z }, { reach * 0.55f, reach * 0.55f, 0.16f }, { NEEDLES[std::min(2, t / 2)], 0.2f, 0.2f }));
  }
  s.push_back(Ball({ 0, 0, 0.92f }, 0.08f, { NEEDLES[2], 0.3f, 0.1f }));
  return s;
}

// A bush: a heap of leaf clumps, some with berries.
std::vector<Solid> Bush(std::mt19937& random, bool berries)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  std::vector<Solid> s;
  for (int i = 0; i < 12; ++i) {
    const float angle = 2.0f * PI * unit(random), out = 0.55f * std::sqrt(unit(random));
    s.push_back(Ball({ std::cos(angle) * out, std::sin(angle) * out, 0.2f + 0.25f * (1.0f - out / 0.55f) }, 0.22f + 0.12f * unit(random),
                     { LEAVES[1 + int(unit(random) * 2.9f)], 0.15f, 0.2f }));
  }
  if (berries) {
    for (int i = 0; i < 9; ++i) {
      const float angle = 2.0f * PI * unit(random), out = 0.5f * std::sqrt(unit(random));
      s.push_back(Ball({ std::cos(angle) * out, std::sin(angle) * out, 0.62f + 0.1f * unit(random) - out * 0.4f }, 0.045f,
                       { BERRY, 0.8f, 0.05f }));
    }
  }
  return s;
}

// A stone, lumpy, moss on its top.
std::vector<Solid> Stone(std::mt19937& random)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  std::vector<Solid> s;
  const Color grey = Scaled(STONE, 0.85f + 0.3f * unit(random));
  s.push_back(Ellipsoid({ 0, 0, 0 }, { 0.7f, 0.55f + 0.15f * unit(random), 0.45f }, { grey, 0.25f, 0.15f }));
  for (int i = 0; i < 3; ++i) {
    const float angle = 2.0f * PI * unit(random);
    s.push_back(Ellipsoid({ std::cos(angle) * 0.35f, std::sin(angle) * 0.3f, 0.0f }, { 0.4f, 0.35f, 0.38f }, { Scaled(grey, 0.95f), 0.25f, 0.15f }));
  }
  s.push_back(Ellipsoid({ -0.15f, -0.12f, 0.08f }, { 0.35f, 0.3f, 0.38f }, { MOSS, 0.05f, 0.3f }));
  return s;
}

}  // namespace

Scenery::~Scenery()
{
  if (this->painted) UnloadRenderTexture(this->canvas);
  for (std::vector<Thing>* things : { &this->leafy, &this->pines, &this->bushes, &this->stones }) {
    for (Thing& thing : *things) {
      UnloadTexture(thing.lit);
      UnloadTexture(thing.shadow);
    }
  }
}

void Scenery::build()
{
  std::mt19937 random(SEED);
  const auto   make = [](const std::vector<Solid>& solids, unsigned seed, int blur) {
    const relief::Picture picture = relief::Build(solids, PICTURE, seed);
    return Thing{ relief::Upload(relief::Light(picture)), relief::Upload(relief::Shadow(picture, blur)) };
  };
  for (int i = 0; i < LEAFY; ++i) this->leafy.push_back(make(Leafy(random), 100 + unsigned(i), 10));
  for (int i = 0; i < PINES; ++i) this->pines.push_back(make(Pine(random), 200 + unsigned(i), 10));
  for (int i = 0; i < BUSHES; ++i) this->bushes.push_back(make(Bush(random, i % 2 == 1), 300 + unsigned(i), 8));
  for (int i = 0; i < STONES; ++i) this->stones.push_back(make(Stone(random), 400 + unsigned(i), 6));
}

void Scenery::prepare(int screenWidth, int screenHeight)
{
  if (this->painted && this->canvas.texture.width == screenWidth && this->canvas.texture.height == screenHeight) return;
  if (this->leafy.empty()) this->build();
  if (this->painted) UnloadRenderTexture(this->canvas);
  this->canvas  = LoadRenderTexture(screenWidth, screenHeight);
  this->painted = true;
  BeginTextureMode(this->canvas);
  this->paint(screenWidth, screenHeight);
  EndTextureMode();
}

void Scenery::draw() const
{
  if (!this->painted) return;
  // A render texture is upside down.
  const Rectangle source = { 0.0f, 0.0f, float(this->canvas.texture.width), -float(this->canvas.texture.height) };
  DrawTextureRec(this->canvas.texture, source, { 0.0f, 0.0f }, WHITE);
}

void Scenery::paint(int screenWidth, int screenHeight)
{
  std::mt19937                          random(SEED);
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  const BoardLayout goban  = LayOutBoard(screenWidth, screenHeight);
  const float       square = float(goban.square);
  const Vector3     light  = relief::LightDirection();

  // ---------------------------------------------------------------- the ground
  // Grass on the clearing and a little round it, its edge ragged; the
  // forest floor beyond. Its height rolls a little, and it is lit as it
  // slopes.
  const int    w = (screenWidth + GROUND_EVERY - 1) / GROUND_EVERY, h = (screenHeight + GROUND_EVERY - 1) / GROUND_EVERY;
  const float  scale = square * 1.2f;  // how far a rise of the ground runs
  const float  rim   = square * 0.6f;  // how far the grass reaches past the goban
  std::vector<float> rise(size_t(w * h));
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      rise[size_t(y * w + x)] = Rolling(float(x * GROUND_EVERY) / scale, float(y * GROUND_EVERY) / scale, SEED);
    }
  }
  Image ground = GenImageColor(w, h, BLANK);
  Color* pixels = static_cast<Color*>(ground.data);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const float X = float(x * GROUND_EVERY), Y = float(y * GROUND_EVERY);
      // How far outside the goban, and so how much grass: all of it on the
      // goban, fading raggedly into the forest floor past its edge.
      const float dx      = std::max({ float(goban.left) - X, X - float(goban.left + goban.side), 0.0f });
      const float dy      = std::max({ float(goban.top) - Y, Y - float(goban.top + goban.side), 0.0f });
      const float out     = std::sqrt(dx * dx + dy * dy);
      const float ragged  = rim * (0.6f + 0.8f * relief::Noise(X / (square * 0.8f), Y / (square * 0.8f), SEED + 5));
      const float grass   = std::clamp(1.0f - (out - ragged) / (square * 0.4f), 0.0f, 1.0f);
      // Grass: each square its own shade, patchy. The floor: dark, with
      // fallen leaves lying about.
      const int   sx      = int(std::floor((X - float(goban.left)) / square)), sy = int(std::floor((Y - float(goban.top)) / square));
      const float own     = relief::Noise(float(sx) * 7.31f, float(sy) * 5.17f, SEED + 9);
      const float patch   = relief::Noise(X / (square * 2.5f), Y / (square * 2.5f), SEED + 11);
      Color       lawn    = Mix(GRASS_DARK, GRASS_LIGHT, 0.25f + 0.35f * own + 0.4f * patch);
      Color       floor   = Mix(FLOOR_DARK, FLOOR_LIGHT, relief::Noise(X / 30.0f, Y / 30.0f, SEED + 13));
      const float litter  = relief::Noise(X / 9.0f, Y / 9.0f, SEED + 17);
      if (litter > 0.62f) floor = Mix(floor, LEAF_LITTER[size_t(relief::Noise(X / 5.0f, Y / 5.0f, SEED + 19) * 3.99f)], (litter - 0.62f) * 3.0f);
      Color colour = Mix(floor, lawn, grass);
      // Lit as it slopes: steeper on the forest floor than on the grass.
      const float steep = 3.0f + 5.0f * (1.0f - grass);
      const float ex    = rise[size_t(y * w + std::min(x + 1, w - 1))] - rise[size_t(y * w + std::max(x - 1, 0))];
      const float ey    = rise[size_t(std::min(y + 1, h - 1) * w + x)] - rise[size_t(std::max(y - 1, 0) * w + x)];
      Vector3     n     = { -ex * steep, -ey * steep, 1.0f };
      const float len   = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
      const float bright = AMBIENT + DIFFUSE * std::max(0.0f, (n.x * light.x + n.y * light.y + n.z * light.z) / len);
      pixels[y * w + x] = Scaled(colour, bright);
    }
  }
  Texture2D groundTexture = LoadTextureFromImage(ground);
  UnloadImage(ground);
  SetTextureFilter(groundTexture, TEXTURE_FILTER_BILINEAR);
  DrawTexturePro(groundTexture, { 0, 0, float(w), float(h) }, { 0, 0, float(w * GROUND_EVERY), float(h * GROUND_EVERY) }, { 0, 0 },
                 0.0f, WHITE);
  // Drawn into the canvas now, so it can go.
  EndTextureMode();
  UnloadTexture(groundTexture);
  BeginTextureMode(this->canvas);

  // ---------------------------------------------------------------- the grass
  // Blades, lit at the tip, and here and there a flower, casting a speck of
  // shadow.
  BeginScissorMode(goban.left, goban.top, goban.side, goban.side);
  const int blades = Goban::SIZE * Goban::SIZE * 16;
  for (int i = 0; i < blades; ++i) {
    const Vector2 at   = { float(goban.left) + unit(random) * float(goban.side), float(goban.top) + unit(random) * float(goban.side) };
    const float   lean = (unit(random) - 0.5f) * square * 0.12f;
    const float   tall = square * (0.06f + 0.08f * unit(random));
    const Vector2 tip  = { at.x + lean, at.y - tall };
    DrawLineEx(at, tip, std::max(1.0f, square * 0.025f), Fade(BLADE_DARK, 0.7f));
    DrawLineEx({ (at.x + tip.x) / 2, (at.y + tip.y) / 2 }, tip, std::max(1.0f, square * 0.02f), Fade(BLADE_LIGHT, 0.55f));
  }
  const int flowers = Goban::SIZE * Goban::SIZE / 5;
  for (int i = 0; i < flowers; ++i) {
    const Vector2 at    = { float(goban.left) + unit(random) * float(goban.side), float(goban.top) + unit(random) * float(goban.side) };
    const Color   petal = FLOWERS[size_t(unit(random) * 3.999f)];
    const float   size  = std::max(1.0f, square * 0.045f);
    DrawCircleV({ at.x + size, at.y + size * 1.2f }, size * 1.6f, Fade(BLACK, 0.18f));
    for (int p = 0; p < 5; ++p) {
      const float angle = 2.0f * PI * float(p) / 5.0f;
      DrawCircleV({ at.x + std::cos(angle) * size, at.y + std::sin(angle) * size }, size, petal);
    }
    DrawCircleV({ at.x - size * 0.3f, at.y - size * 0.3f }, size * 0.35f, Fade(WHITE, 0.6f));
    DrawCircleV(at, size * 0.8f, FLOWERS[1]);
  }
  EndScissorMode();

  // ---------------------------------------------------------------- what stands on it
  // Trees round the clearing, bushes and stones at its edge; never over the
  // goban's squares. All the shadows first, then everything, the far ones
  // first so the near ones lie over them.
  struct Placed {
    const Thing* thing;
    Vector2      at;
    float        size;   // across, in pixels
    float        casts;  // how far its shadow falls, as a share of its size
  };
  std::vector<Placed> placed;
  const Rectangle board = { float(goban.left), float(goban.top), float(goban.side), float(goban.side) };
  const auto      apart = [&](Vector2 at, float r) {
    const float nx = std::clamp(at.x, board.x, board.x + board.width), ny = std::clamp(at.y, board.y, board.y + board.height);
    return std::hypot(at.x - nx, at.y - ny) - r;
  };
  // Bushes and stones along the clearing's edge.
  for (int i = 0; i < 70; ++i) {
    const bool  stone = unit(random) < 0.4f;
    const float size  = square * (stone ? 0.5f + 0.4f * unit(random) : 0.9f + 0.6f * unit(random));
    const float side  = unit(random) * 4.0f;
    const float along = unit(random);
    const float off   = rim * (0.3f + 1.4f * unit(random)) + size * 0.5f;
    Vector2     at;
    if (side < 1)      at = { board.x + along * board.width, board.y - off };
    else if (side < 2) at = { board.x + along * board.width, board.y + board.height + off };
    else if (side < 3) at = { board.x - off, board.y + along * board.height };
    else               at = { board.x + board.width + off, board.y + along * board.height };
    if (apart(at, size * 0.45f) < 0.0f) continue;
    const std::vector<Thing>& kind = stone ? this->stones : this->bushes;
    placed.push_back({ &kind[size_t(unit(random) * float(kind.size())) % kind.size()], at, size, stone ? 0.08f : 0.13f });
  }
  // Trees everywhere else, the forest.
  const int tries = int(float(screenWidth) * float(screenHeight) / (square * square) * 0.9f);
  for (int i = 0; i < tries; ++i) {
    const bool  pine = unit(random) < 0.35f;
    const float size = square * (pine ? 2.2f + 1.4f * unit(random) : 2.6f + 1.8f * unit(random));
    const Vector2 at = { -size * 0.5f + unit(random) * (float(screenWidth) + size), -size * 0.5f + unit(random) * (float(screenHeight) + size) };
    if (apart(at, size * 0.42f) < rim * 1.2f) continue;
    const std::vector<Thing>& kind = pine ? this->pines : this->leafy;
    placed.push_back({ &kind[size_t(unit(random) * float(kind.size())) % kind.size()], at, size, 0.2f });
  }
  std::sort(placed.begin(), placed.end(), [](const Placed& a, const Placed& b) { return a.size < b.size || (a.size == b.size && a.at.y < b.at.y); });

  // Not turned: the light is worked into them, from where it comes.
  const auto draw = [](const Texture2D& texture, Vector2 at, float size, Color tint) {
    DrawTexturePro(texture, { 0, 0, float(texture.width), float(texture.height) }, { at.x, at.y, size, size },
                   { size / 2.0f, size / 2.0f }, 0.0f, tint);
  };
  // Shadows fall away from the light: down and to the right.
  for (const Placed& p : placed) {
    draw(p.thing->shadow, { p.at.x + p.size * p.casts * 0.75f, p.at.y + p.size * p.casts }, p.size * 1.05f, Fade(WHITE, 0.55f));
  }
  for (const Placed& p : placed) draw(p.thing->lit, p.at, p.size, WHITE);
}

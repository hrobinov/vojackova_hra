#include <game/Scenery.hpp>

#include <game/BoardView.hpp>
#include <game/Goban.hpp>

#include <algorithm>
#include <cmath>
#include <random>

namespace {

// The same forest every time.
constexpr unsigned SEED = 20260930;

// The grass of the squares, and the forest floor round them.
constexpr Color GRASS_DARK  = Color{ 86, 138, 62, 255 };
constexpr Color GRASS_LIGHT = Color{ 108, 160, 74, 255 };
constexpr Color BLADE_DARK  = Color{ 62, 110, 46, 255 };
constexpr Color BLADE_LIGHT = Color{ 140, 186, 92, 255 };
constexpr Color FLOOR       = Color{ 38, 62, 34, 255 };
constexpr Color FLOOR_SPECK = Color{ 58, 84, 44, 255 };
constexpr Color CLEARING_RIM = Color{ 52, 82, 40, 255 };

// Broad-leaved trees, darkest to lightest; pines, darkest to lightest.
constexpr Color LEAVES[]  = { { 34, 78, 38, 255 }, { 48, 104, 48, 255 }, { 70, 132, 60, 255 }, { 104, 164, 80, 255 } };
constexpr Color NEEDLES[] = { { 22, 58, 40, 255 }, { 30, 76, 50, 255 }, { 46, 100, 64, 255 } };
constexpr Color FLOWERS[] = { { 245, 240, 230, 255 }, { 250, 214, 70, 255 }, { 230, 120, 170, 255 }, { 150, 170, 250, 255 } };

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

Color Mix(Color a, Color b, float t)
{
  return Color{ Byte(a.r + (b.r - a.r) * t), Byte(a.g + (b.g - a.g) * t), Byte(a.b + (b.b - a.b) * t),
                Byte(a.a + (b.a - a.a) * t) };
}

Color WithAlpha(Color color, float alpha)
{
  color.a = Byte(alpha * 255.0f);
  return color;
}

// A broad-leaved tree seen from above, `r` across to its edge: its shadow,
// a dark crown, lighter clumps of leaves round it, and the light catching
// the top left of it.
void LeafyTree(std::mt19937& random, Vector2 at, float r)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  DrawCircleV({ at.x + r * 0.3f, at.y + r * 0.35f }, r * 1.02f, WithAlpha(BLACK, 0.28f));
  DrawCircleV(at, r, LEAVES[0]);
  const int clumps = 6 + int(unit(random) * 3);
  for (int i = 0; i < clumps; ++i) {
    const float angle = 2.0f * PI * (float(i) + unit(random) * 0.6f) / float(clumps);
    const Vector2 c   = { at.x + std::cos(angle) * r * 0.55f, at.y + std::sin(angle) * r * 0.55f };
    DrawCircleV(c, r * (0.38f + 0.1f * unit(random)), LEAVES[1]);
  }
  DrawCircleV({ at.x - r * 0.12f, at.y - r * 0.12f }, r * 0.55f, LEAVES[1]);
  DrawCircleV({ at.x - r * 0.25f, at.y - r * 0.28f }, r * 0.36f, LEAVES[2]);
  DrawCircleV({ at.x - r * 0.35f, at.y - r * 0.38f }, r * 0.16f, LEAVES[3]);
}

// A pine seen from above: its shadow, and star-shaped layers of needles
// getting smaller and lighter towards the tip.
void Pine(std::mt19937& random, Vector2 at, float r)
{
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  DrawCircleV({ at.x + r * 0.3f, at.y + r * 0.35f }, r * 0.95f, WithAlpha(BLACK, 0.3f));
  const float turn = 360.0f * unit(random);
  for (int layer = 0; layer < 3; ++layer) {
    const float size = r * (1.0f - 0.28f * float(layer));
    // A star: a many-sided polygon, and a smaller one turned half a point.
    DrawPoly(at, 9, size * 0.72f, turn + 20.0f * float(layer), NEEDLES[layer]);
    for (int point = 0; point < 9; ++point) {
      const float angle = (turn + 20.0f * float(layer) + 40.0f * float(point)) * DEG2RAD;
      DrawTriangle(at, { at.x + std::cos(angle + 0.2f) * size * 0.7f, at.y + std::sin(angle + 0.2f) * size * 0.7f },
                   { at.x + std::cos(angle) * size, at.y + std::sin(angle) * size }, NEEDLES[layer]);
      DrawTriangle(at, { at.x + std::cos(angle) * size, at.y + std::sin(angle) * size },
                   { at.x + std::cos(angle - 0.2f) * size * 0.7f, at.y + std::sin(angle - 0.2f) * size * 0.7f }, NEEDLES[layer]);
    }
  }
  DrawCircleV(at, r * 0.12f, NEEDLES[2]);
}

}  // namespace

Scenery::~Scenery()
{
  if (this->painted) UnloadRenderTexture(this->canvas);
}

void Scenery::prepare(int screenWidth, int screenHeight)
{
  if (this->painted && this->canvas.texture.width == screenWidth && this->canvas.texture.height == screenHeight) return;
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

  // The forest floor, speckled with fallen needles and leaves.
  ClearBackground(FLOOR);
  const int specks = screenWidth * screenHeight / 60;
  for (int i = 0; i < specks; ++i) {
    DrawPixelV({ unit(random) * float(screenWidth), unit(random) * float(screenHeight) },
               Mix(FLOOR, FLOOR_SPECK, unit(random)));
  }

  // The clearing: its rim of lighter undergrowth, and the grass.
  const float rim = square * 0.35f;
  DrawRectangleRounded({ float(goban.left) - rim, float(goban.top) - rim, float(goban.side) + 2 * rim, float(goban.side) + 2 * rim },
                       0.05f, 8, CLEARING_RIM);
  for (int y = 0; y < Goban::SIZE; ++y) {
    for (int x = 0; x < Goban::SIZE; ++x) {
      DrawRectangle(goban.left + x * goban.square, goban.top + y * goban.square, goban.square + goban.line,
                    goban.square + goban.line, Mix(GRASS_DARK, GRASS_LIGHT, 0.3f + 0.5f * unit(random)));
    }
  }
  // Darker and lighter patches across the squares, so the grass isn't a
  // chessboard of shades.
  BeginScissorMode(goban.left, goban.top, goban.side, goban.side);
  for (int i = 0; i < 40; ++i) {
    const Vector2 at = { float(goban.left) + unit(random) * float(goban.side), float(goban.top) + unit(random) * float(goban.side) };
    const Color   tone = unit(random) < 0.6f ? BLADE_DARK : BLADE_LIGHT;
    DrawCircleGradient(int(at.x), int(at.y), square * (1.0f + 2.0f * unit(random)), WithAlpha(tone, 0.22f), WithAlpha(tone, 0.0f));
  }
  // Blades of grass, and here and there a flower.
  const int blades = Goban::SIZE * Goban::SIZE * 14;
  for (int i = 0; i < blades; ++i) {
    const Vector2 at   = { float(goban.left) + unit(random) * float(goban.side), float(goban.top) + unit(random) * float(goban.side) };
    const float   lean = (unit(random) - 0.5f) * square * 0.12f;
    const float   tall = square * (0.07f + 0.08f * unit(random));
    DrawLineEx(at, { at.x + lean, at.y - tall }, std::max(1.0f, square * 0.025f),
               WithAlpha(unit(random) < 0.55f ? BLADE_DARK : BLADE_LIGHT, 0.8f));
  }
  const int flowers = Goban::SIZE * Goban::SIZE / 5;
  for (int i = 0; i < flowers; ++i) {
    const Vector2 at    = { float(goban.left) + unit(random) * float(goban.side), float(goban.top) + unit(random) * float(goban.side) };
    const Color   petal = FLOWERS[size_t(unit(random) * 3.999f)];
    const float   size  = std::max(1.0f, square * 0.045f);
    for (int p = 0; p < 5; ++p) {
      const float angle = 2.0f * PI * float(p) / 5.0f;
      DrawCircleV({ at.x + std::cos(angle) * size, at.y + std::sin(angle) * size }, size, petal);
    }
    DrawCircleV(at, size * 0.8f, FLOWERS[1]);
  }
  EndScissorMode();

  // Trees all round the clearing, the far ones first so the near ones'
  // crowns lie over them -- never reaching over its squares.
  struct Tree {
    Vector2 at;
    float   r;
    bool    pine;
  };
  std::vector<Tree> trees;
  const Rectangle clearing = { float(goban.left) - rim, float(goban.top) - rim, float(goban.side) + 2 * rim, float(goban.side) + 2 * rim };
  const int       tries    = int(float(screenWidth) * float(screenHeight) / (square * square) * 1.2f);
  for (int i = 0; i < tries; ++i) {
    const float r  = square * (0.7f + 0.9f * unit(random));
    const Vector2 at = { -r + unit(random) * (float(screenWidth) + 2 * r), -r + unit(random) * (float(screenHeight) + 2 * r) };
    // Not over the clearing: the nearest point of it is further than the crown.
    const float nx = std::clamp(at.x, clearing.x, clearing.x + clearing.width);
    const float ny = std::clamp(at.y, clearing.y, clearing.y + clearing.height);
    if (std::hypot(at.x - nx, at.y - ny) < r * 0.9f) continue;
    trees.push_back({ at, r, unit(random) < 0.35f });
  }
  std::sort(trees.begin(), trees.end(), [](const Tree& a, const Tree& b) { return a.at.y < b.at.y; });
  for (const Tree& tree : trees) {
    if (tree.pine) Pine(random, tree.at, tree.r);
    else           LeafyTree(random, tree.at, tree.r);
  }
}

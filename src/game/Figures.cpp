#include <game/Figures.hpp>

#include <game/Relief.hpp>

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace figures {

namespace {

using relief::Ball;
using relief::Capsule;
using relief::Ellipsoid;
using relief::Material;
using relief::Pattern;
using relief::Solid;
using relief::Stain;

// How many pixels across each model is worked out at, and how far its
// shadow is blurred.
constexpr int PICTURE = 320;
constexpr int BLUR    = 9;

// How far the shadow falls from the figure, in squares, away from the light,
// and how dark it is.
constexpr Vector2 SHADOW_FALLS = { 0.1f, 0.13f };
constexpr float   SHADOW_DARK  = 0.5f;

// ---------------------------------------------------------------- materials

// The soldier: fabric that wrinkles, a scratched helmet.
constexpr Material UNIFORM  = { .color = { 58, 98, 188, 255 }, .shine = 0.12f, .grain = 0.08f, .rough = 0.45f, .wrinkles = 14.0f };
constexpr Material UNIFORM2 = { .color = { 44, 76, 150, 255 }, .shine = 0.12f, .grain = 0.08f, .rough = 0.45f, .wrinkles = 16.0f };
constexpr Material TROUSERS = { .color = { 46, 58, 96, 255 }, .shine = 0.1f, .grain = 0.08f, .rough = 0.45f, .wrinkles = 14.0f };
constexpr Material HELMET   = { .color = { 52, 78, 132, 255 }, .shine = 0.55f, .grain = 0.05f, .rough = 0.12f, .wrinkles = 70.0f };
constexpr Material HELMET2  = { .color = { 38, 58, 100, 255 }, .shine = 0.45f, .grain = 0.05f, .rough = 0.12f, .wrinkles = 70.0f };
constexpr Material SKIN     = { .color = { 232, 184, 146, 255 }, .shine = 0.25f, .grain = 0.04f, .rough = 0.15f, .wrinkles = 60.0f };
constexpr Material HAIR     = { .color = { 92, 60, 34, 255 }, .shine = 0.4f, .grain = 0.15f, .rough = 0.8f, .wrinkles = 70.0f };
constexpr Material PACK     = { .color = { 112, 92, 60, 255 }, .shine = 0.1f, .grain = 0.12f, .rough = 0.5f, .wrinkles = 18.0f,
                                .pattern = Pattern::Stitches, .second = { 70, 56, 36, 255 }, .weave = 6.0f };
constexpr Material ROLL     = { .color = { 150, 120, 74, 255 }, .shine = 0.1f, .grain = 0.14f, .rough = 0.5f, .wrinkles = 24.0f,
                                .pattern = Pattern::Stripes, .second = { 124, 96, 58, 255 }, .weave = 18.0f };
constexpr Material STRAP    = { .color = { 70, 54, 36, 255 }, .shine = 0.2f, .grain = 0.06f };
constexpr Material METAL    = { .color = { 50, 52, 58, 255 }, .shine = 0.8f, .grain = 0.03f, .stains = false };
constexpr Material WOOD     = { .color = { 128, 80, 44, 255 }, .shine = 0.35f, .grain = 0.1f, .pattern = Pattern::Stripes,
                                .second = { 108, 66, 36, 255 }, .weave = 40.0f };
constexpr Material BOOT     = { .color = { 38, 32, 30, 255 }, .shine = 0.35f, .grain = 0.05f, .rough = 0.3f, .wrinkles = 30.0f };

// The zombies, as in 7 Days to Die: grey, sickly skin, veins dark through
// it; raw flesh and bone where it is torn; clothes of the people they were.
constexpr Material ZSKIN    = { .color = { 148, 148, 122, 255 }, .shine = 0.25f, .grain = 0.12f, .rough = 0.35f, .wrinkles = 45.0f,
                                .pattern = Pattern::Veins, .second = { 92, 74, 90, 255 }, .weave = 5.0f };
constexpr Material ZFERAL   = { .color = { 112, 102, 92, 255 }, .shine = 0.3f, .grain = 0.14f, .rough = 0.45f, .wrinkles = 50.0f,
                                .pattern = Pattern::Veins, .second = { 50, 30, 36, 255 }, .weave = 6.0f };
constexpr Material FLESH    = { .color = { 150, 42, 38, 255 }, .shine = 0.7f, .grain = 0.2f, .rough = 0.6f, .wrinkles = 60.0f, .stains = false };
constexpr Material BONE     = { .color = { 214, 204, 176, 255 }, .shine = 0.4f, .grain = 0.08f, .rough = 0.2f, .wrinkles = 40.0f };
constexpr Material SOCKET   = { .color = { 38, 24, 24, 255 }, .shine = 0.1f, .grain = 0.05f, .stains = false };
constexpr Material MILKY    = { .color = { 206, 200, 164, 255 }, .shine = 0.75f, .grain = 0.04f, .stains = false };
// Glowing: anything as shiny as this shines of itself, whatever the light.
constexpr Material GLOWING  = { .color = { 255, 46, 30, 255 }, .shine = 1.0f, .grain = 0.02f, .stains = false };
constexpr Material EMBERS   = { .color = { 255, 150, 30, 255 }, .shine = 1.0f, .grain = 0.02f, .stains = false };
// The red zombie's: skin flayed raw, a fireman's coat with its stripes, and
// his helmet.
constexpr Material FLAYED   = { .color = { 156, 58, 50, 255 }, .shine = 0.55f, .grain = 0.2f, .rough = 0.55f, .wrinkles = 55.0f,
                                .pattern = Pattern::Veins, .second = { 70, 12, 16, 255 }, .weave = 7.0f };
constexpr Material FIRECOAT = { .color = { 150, 34, 26, 255 }, .shine = 0.2f, .grain = 0.12f, .rough = 0.5f, .wrinkles = 11.0f,
                                .pattern = Pattern::Stripes, .second = { 206, 200, 120, 255 }, .weave = 3.0f, .band = 0.14f };
constexpr Material FIREPANTS = { .color = { 70, 60, 44, 255 }, .shine = 0.15f, .grain = 0.12f, .rough = 0.5f, .wrinkles = 12.0f };
constexpr Material FIREHAT  = { .color = { 128, 24, 20, 255 }, .shine = 0.6f, .grain = 0.06f, .rough = 0.15f, .wrinkles = 40.0f };
constexpr Material FIREHAT2 = { .color = { 40, 30, 28, 255 }, .shine = 0.5f, .grain = 0.06f };
constexpr Material DEADEYE  = { .color = { 110, 104, 84, 255 }, .shine = 0.3f, .grain = 0.02f, .stains = false };
constexpr Material MOUTH    = { .color = { 58, 16, 16, 255 }, .shine = 0.6f, .grain = 0.05f, .stains = false };
constexpr Material TEETH    = { .color = { 206, 190, 140, 255 }, .shine = 0.5f, .grain = 0.1f, .stains = false };

// Clothes.
constexpr Material FLANNEL  = { .color = { 148, 38, 34, 255 }, .shine = 0.08f, .grain = 0.1f, .rough = 0.5f, .wrinkles = 14.0f,
                                .pattern = Pattern::Plaid, .second = { 34, 28, 34, 255 }, .weave = 9.0f };
constexpr Material DENIM    = { .color = { 62, 84, 122, 255 }, .shine = 0.1f, .grain = 0.1f, .rough = 0.45f, .wrinkles = 16.0f,
                                .pattern = Pattern::Denim, .second = { 40, 54, 86, 255 }, .weave = 10.0f };
constexpr Material TSHIRT   = { .color = { 136, 134, 126, 255 }, .shine = 0.06f, .grain = 0.12f, .rough = 0.6f, .wrinkles = 12.0f };
constexpr Material SCRUBS   = { .color = { 100, 148, 166, 255 }, .shine = 0.1f, .grain = 0.08f, .rough = 0.45f, .wrinkles = 13.0f };
constexpr Material SUIT     = { .color = { 46, 48, 58, 255 }, .shine = 0.15f, .grain = 0.08f, .rough = 0.4f, .wrinkles = 14.0f,
                                .pattern = Pattern::Stripes, .second = { 56, 58, 70, 255 }, .weave = 30.0f };
constexpr Material COLLAR   = { .color = { 206, 204, 196, 255 }, .shine = 0.2f, .grain = 0.06f, .rough = 0.3f, .wrinkles = 20.0f };
constexpr Material TIE      = { .color = { 132, 22, 28, 255 }, .shine = 0.4f, .grain = 0.05f, .pattern = Pattern::Stripes,
                                .second = { 92, 14, 20, 255 }, .weave = 40.0f };
constexpr Material HOODIE   = { .color = { 34, 34, 38, 255 }, .shine = 0.08f, .grain = 0.12f, .rough = 0.55f, .wrinkles = 10.0f };
constexpr Material JEANS    = { .color = { 48, 62, 92, 255 }, .shine = 0.08f, .grain = 0.1f, .rough = 0.45f, .wrinkles = 14.0f,
                                .pattern = Pattern::Denim, .second = { 34, 44, 70, 255 }, .weave = 10.0f };
constexpr Material SLACKS   = { .color = { 40, 42, 50, 255 }, .shine = 0.12f, .grain = 0.08f, .rough = 0.4f, .wrinkles = 14.0f };
constexpr Material SHOE     = { .color = { 52, 38, 28, 255 }, .shine = 0.35f, .grain = 0.08f, .rough = 0.3f, .wrinkles = 30.0f };
constexpr Material CAP      = { .color = { 78, 96, 58, 255 }, .shine = 0.1f, .grain = 0.14f, .rough = 0.4f, .wrinkles = 20.0f,
                                .pattern = Pattern::Stitches, .second = { 58, 72, 44, 255 }, .weave = 12.0f };
constexpr Material CAP_VISOR = { .color = { 52, 64, 38, 255 }, .shine = 0.2f, .grain = 0.1f, .rough = 0.3f, .wrinkles = 24.0f };
constexpr Material CAP_PATCH = { .color = { 214, 180, 60, 255 }, .shine = 0.3f, .grain = 0.1f };
constexpr Material BUTTON   = { .color = { 180, 170, 150, 255 }, .shine = 0.6f, .grain = 0.02f };
constexpr Material GREY     = { .color = { 124, 120, 114, 255 }, .shine = 0.35f, .grain = 0.18f, .rough = 0.85f, .wrinkles = 80.0f };
constexpr Material BLOND    = { .color = { 156, 128, 80, 255 }, .shine = 0.4f, .grain = 0.18f, .rough = 0.85f, .wrinkles = 80.0f };
constexpr Material BROWN_HAIR    = { .color = { 70, 52, 38, 255 }, .shine = 0.35f, .grain = 0.18f, .rough = 0.85f, .wrinkles = 80.0f };
constexpr Material BLACKHAIR = { .color = { 22, 20, 24, 255 }, .shine = 0.6f, .grain = 0.15f, .rough = 0.85f, .wrinkles = 80.0f };

// The stains.
constexpr Color BLOOD = { 104, 10, 8, 255 };
constexpr Color DRIED = { 70, 20, 14, 255 };
constexpr Color DIRT  = { 72, 58, 40, 255 };
constexpr Color ROT   = { 90, 100, 52, 255 };

// ---------------------------------------------------------------- zombies

// Who a zombie was: what it wears, how its head looks, how it was hurt.
enum class Head { Bald, Cap, LongHair, Grey, Hood, Helmet };
struct Kind {
  Material skin;
  Material shirt;
  bool     longSleeves;
  Material legs;
  Head     head;
  Material hair;
  Material eyes;
  bool     ribs = false;    // torn open on its back, the ribs showing
  bool     straps = false;  // overalls over the shirt
  bool     tie = false;     // a suit: a white collar, and a tie
  float    reach = 0.0f;    // one arm further out than the other
};

// The white zombies' four, and the black one: a feral, in a hoodie.
const Kind FARMER   = { ZSKIN, FLANNEL, true, DENIM, Head::Cap, BROWN_HAIR, MILKY, false, true, false, 0.06f };
const Kind DRIFTER  = { ZSKIN, TSHIRT, false, JEANS, Head::Bald, GREY, MILKY, true, false, false, -0.05f };
const Kind NURSE    = { ZSKIN, SCRUBS, false, SCRUBS, Head::LongHair, BLOND, MILKY, false, false, false, 0.03f };
const Kind BUSINESS = { ZSKIN, SUIT, true, SLACKS, Head::Grey, GREY, MILKY, false, false, true, -0.04f };
const Kind FERAL    = { ZFERAL, HOODIE, true, JEANS, Head::Hood, BLACKHAIR, GLOWING, false, false, false, 0.08f };
// And the red one: a fireman, burnt and flayed, his coat and helmet on.
const Kind FIREMAN  = { FLAYED, FIRECOAT, true, FIREPANTS, Head::Helmet, BLACKHAIR, EMBERS, true, false, false, -0.06f };

struct Figure {
  std::vector<Solid> solids;
  std::vector<Stain> stains;
};

// A hand at `wrist`, reaching the way `forward` points (a unit long), the
// fingers curled like claws, the thumb on the side `inner` (+1 or -1).
void Hand(std::vector<Solid>& s, Vector3 wrist, Vector2 forward, float inner, const Material& skin)
{
  const Vector2 side = { -forward.y, forward.x };
  const auto    at   = [&](float ahead, float aside, float up) {
    return Vector3{ wrist.x + forward.x * ahead + side.x * aside, wrist.y + forward.y * ahead + side.y * aside, wrist.z + up };
  };
  s.push_back(Ellipsoid(at(0.05f, 0.0f, 0.0f), { 0.06f, 0.055f, 0.035f }, skin));
  for (const float finger : { -0.042f, -0.014f, 0.014f, 0.042f }) {
    const float longer = 0.02f - std::abs(finger) * 0.3f;
    s.push_back(Capsule(at(0.09f, finger, 0.005f), at(0.16f + longer, finger * 1.15f, 0.0f), 0.018f, skin));
    s.push_back(Capsule(at(0.16f + longer, finger * 1.15f, 0.0f), at(0.2f + longer, finger * 1.2f, -0.03f), 0.015f, skin));
  }
  s.push_back(Capsule(at(0.04f, inner * 0.05f, 0.0f), at(0.1f, inner * 0.08f, 0.01f), 0.019f, skin));
}

// A zombie standing, facing +x: hunched, both arms reaching for you.
Figure Standing(const Kind& k, unsigned seed)
{
  Figure f;
  std::vector<Solid>& s = f.solids;
  const Material&     sleeve = k.longSleeves ? k.shirt : k.skin;

  // Hips under it, the hunched back, the shoulder blades and shoulders.
  s.push_back(Ellipsoid({ -0.22f, 0.0f, 0.1f }, { 0.2f, 0.32f, 0.2f }, k.legs));
  s.push_back(Ellipsoid({ -0.06f, 0.0f, 0.22f }, { 0.27f, 0.43f, 0.3f }, k.shirt));
  s.push_back(Ellipsoid({ -0.02f, 0.0f, 0.34f }, { 0.17f, 0.3f, 0.19f }, k.shirt));
  for (const float side : { -1.0f, 1.0f }) {
    s.push_back(Ellipsoid({ -0.14f, side * 0.17f, 0.42f }, { 0.12f, 0.1f, 0.1f }, k.shirt));
    s.push_back(Ellipsoid({ 0.0f, side * 0.39f, 0.3f }, { 0.15f, 0.14f, 0.18f }, k.shirt));
  }
  // Where the shirt is torn through, skin.
  s.push_back(Ellipsoid({ -0.18f, 0.24f, 0.44f }, { 0.07f, 0.05f, 0.08f }, k.skin));
  s.push_back(Ellipsoid({ -0.04f, -0.3f, 0.4f }, { 0.05f, 0.04f, 0.1f }, k.skin));

  if (k.straps) {
    for (const float side : { -1.0f, 1.0f }) {
      s.push_back(Capsule({ -0.32f, side * 0.17f, 0.46f }, { 0.08f, side * 0.2f, 0.5f }, 0.036f, DENIM));
      s.push_back(Ball({ 0.08f, side * 0.2f, 0.52f }, 0.02f, BUTTON));
    }
  }
  if (k.tie) {
    s.push_back(Capsule({ 0.04f, -0.13f, 0.5f }, { 0.08f, 0.0f, 0.52f }, 0.035f, COLLAR));
    s.push_back(Capsule({ 0.08f, 0.0f, 0.52f }, { 0.04f, 0.13f, 0.5f }, 0.035f, COLLAR));
    s.push_back(Ellipsoid({ 0.2f, 0.02f, 0.4f }, { 0.08f, 0.03f, 0.1f }, TIE));
  }
  if (k.ribs) {
    // Torn open: raw flesh, the ribs across it, a stretch of spine.
    s.push_back(Ellipsoid({ -0.14f, -0.13f, 0.46f }, { 0.13f, 0.1f, 0.08f }, FLESH));
    for (int rib = 0; rib < 3; ++rib) {
      const float x = -0.22f + 0.06f * float(rib);
      s.push_back(Capsule({ x, -0.21f, 0.5f }, { x + 0.02f, -0.05f, 0.52f }, 0.017f, BONE));
    }
    s.push_back(Capsule({ -0.3f, -0.01f, 0.49f }, { -0.06f, -0.01f, 0.5f }, 0.024f, BONE));
    f.stains.push_back({ { -0.14f, -0.13f }, 0.17f, BLOOD, 0.75f });
  }

  // The arms, reaching: upper arm, forearm (a cuff where the sleeve ends),
  // and the hand.
  for (const float side : { -1.0f, 1.0f }) {
    const float  out   = 0.62f + (side > 0 ? k.reach : -k.reach);
    const float  y     = side * 0.33f;
    const Vector3 elbow = { 0.28f, side * 0.35f, 0.4f };
    const Vector3 wrist = { out, y * 0.9f, 0.44f };
    s.push_back(Capsule({ 0.02f, side * 0.39f, 0.34f }, elbow, 0.092f, sleeve));
    if (k.longSleeves) {
      const Vector3 cuff = { elbow.x + (wrist.x - elbow.x) * 0.6f, elbow.y + (wrist.y - elbow.y) * 0.6f, 0.43f };
      s.push_back(Capsule(elbow, cuff, 0.078f, k.shirt));
      s.push_back(Capsule(cuff, wrist, 0.06f, k.skin));
    } else {
      s.push_back(Capsule(elbow, wrist, 0.068f, k.skin));
    }
    const float dx = wrist.x - elbow.x, dy = wrist.y - elbow.y, len = std::sqrt(dx * dx + dy * dy);
    Hand(s, wrist, { dx / len, dy / len }, -side, k.skin);
  }

  // The neck, and the head: skull, cheekbones, brow, a jaw hanging open;
  // the eyes sunk in their sockets, the nose rotted away to a hole, the
  // teeth bared between torn lips; ears.
  s.push_back(Capsule({ -0.02f, 0.0f, 0.44f }, { 0.06f, 0.0f, 0.5f }, 0.09f, k.skin));
  s.push_back(Ellipsoid({ 0.05f, 0.0f, 0.54f }, { 0.22f, 0.2f, 0.24f }, k.skin));
  for (const float side : { -1.0f, 1.0f }) {
    s.push_back(Ellipsoid({ 0.06f, side * 0.2f, 0.5f }, { 0.05f, 0.035f, 0.1f }, k.skin));
    s.push_back(Ellipsoid({ 0.2f, side * 0.1f, 0.54f }, { 0.07f, 0.06f, 0.08f }, k.skin));
    s.push_back(Ball({ 0.235f, side * 0.075f, 0.62f }, 0.05f, SOCKET));
    s.push_back(Ball({ 0.245f, side * 0.075f, 0.655f }, 0.028f, k.eyes));
  }
  s.push_back(Capsule({ 0.2f, -0.12f, 0.67f }, { 0.2f, 0.12f, 0.67f }, 0.038f, k.skin));
  s.push_back(Ellipsoid({ 0.27f, 0.0f, 0.5f }, { 0.08f, 0.11f, 0.12f }, k.skin));
  s.push_back(Ellipsoid({ 0.285f, 0.0f, 0.585f }, { 0.03f, 0.025f, 0.05f }, SOCKET));
  s.push_back(Ellipsoid({ 0.32f, 0.0f, 0.5f }, { 0.035f, 0.08f, 0.08f }, MOUTH));
  s.push_back(Capsule({ 0.335f, -0.055f, 0.52f }, { 0.335f, 0.055f, 0.52f }, 0.02f, TEETH));

  // The top of the head.
  switch (k.head) {
  case Head::Bald:
    f.stains.push_back({ { 0.02f, 0.08f }, 0.1f, ROT, 0.3f, 0.8f });
    break;
  case Head::Cap:
    s.push_back(Ellipsoid({ 0.02f, 0.0f, 0.6f }, { 0.22f, 0.21f, 0.2f }, CAP));
    s.push_back(Ellipsoid({ 0.22f, 0.0f, 0.72f }, { 0.13f, 0.17f, 0.03f }, CAP_VISOR));
    s.push_back(Ellipsoid({ 0.15f, 0.0f, 0.73f }, { 0.045f, 0.065f, 0.03f }, CAP_PATCH));
    s.push_back(Ball({ 0.02f, 0.0f, 0.795f }, 0.02f, CAP_VISOR));
    for (const float side : { -1.0f, 1.0f }) s.push_back(Ellipsoid({ -0.08f, side * 0.18f, 0.6f }, { 0.07f, 0.04f, 0.08f }, k.hair));
    break;
  case Head::LongHair:
    s.push_back(Ellipsoid({ -0.02f, 0.0f, 0.62f }, { 0.23f, 0.22f, 0.19f }, k.hair));
    for (const float side : { -1.0f, 1.0f }) {
      s.push_back(Capsule({ -0.05f, side * 0.14f, 0.6f }, { -0.3f, side * 0.18f, 0.46f }, 0.06f, k.hair));
      s.push_back(Capsule({ 0.08f, side * 0.16f, 0.6f }, { -0.1f, side * 0.26f, 0.44f }, 0.045f, k.hair));
    }
    break;
  case Head::Grey:
    // Thinning: tufts round the sides and back, the crown bare.
    for (int t = 0; t < 7; ++t) {
      const float angle = PI * (0.55f + 0.9f * float(t) / 6.0f);
      s.push_back(Ellipsoid({ 0.02f + std::cos(angle) * 0.15f, std::sin(angle) * 0.16f, 0.62f }, { 0.07f, 0.06f, 0.14f }, k.hair));
    }
    f.stains.push_back({ { 0.05f, -0.05f }, 0.08f, ROT, 0.3f, 0.7f });
    break;
  case Head::Hood:
    // The hood down behind the head, the hair black and matted over it.
    s.push_back(Ellipsoid({ -0.16f, 0.0f, 0.5f }, { 0.2f, 0.27f, 0.2f }, HOODIE));
    s.push_back(Ellipsoid({ -0.02f, 0.0f, 0.6f }, { 0.21f, 0.21f, 0.2f }, k.hair));
    for (const float side : { -1.0f, 1.0f }) s.push_back(Ellipsoid({ 0.14f, side * 0.13f, 0.64f }, { 0.09f, 0.05f, 0.12f }, k.hair));
    break;
  case Head::Helmet:
    // A fireman's helmet: the long brim behind, the dome, the ridge along
    // its top, a badge on the front.
    s.push_back(Ellipsoid({ -0.06f, 0.0f, 0.62f }, { 0.33f, 0.27f, 0.05f }, FIREHAT2));
    s.push_back(Ellipsoid({ 0.0f, 0.0f, 0.6f }, { 0.22f, 0.21f, 0.22f }, FIREHAT));
    s.push_back(Capsule({ -0.16f, 0.0f, 0.8f }, { 0.14f, 0.0f, 0.8f }, 0.035f, FIREHAT));
    s.push_back(Ellipsoid({ 0.19f, 0.0f, 0.72f }, { 0.04f, 0.06f, 0.06f }, { .color = { 200, 170, 60, 255 }, .shine = 0.8f, .grain = 0.04f }));
    break;
  }

  // Blood and dirt: down the back and shoulders, on the hands and the face.
  const float spread = float(seed % 7) * 0.02f;
  f.stains.push_back({ { -0.1f + spread, 0.18f }, 0.16f, DRIED, 0.3f, 0.85f });
  f.stains.push_back({ { 0.02f, -0.36f + spread }, 0.12f, BLOOD, 0.75f, 0.85f });
  f.stains.push_back({ { 0.33f, 0.0f }, 0.05f, BLOOD, 0.8f, 0.9f });
  f.stains.push_back({ { 0.68f, 0.3f }, 0.08f, BLOOD, 0.75f, 0.8f });
  f.stains.push_back({ { 0.66f, -0.3f }, 0.08f, DIRT, 0.1f, 0.8f });
  f.stains.push_back({ { -0.3f, -0.1f }, 0.18f, DIRT, 0.05f, 0.6f });
  // Rotting in patches on the arms and the head.
  f.stains.push_back({ { 0.5f, 0.31f }, 0.06f, ROT, 0.3f, 0.7f });
  f.stains.push_back({ { 0.44f, -0.3f }, 0.05f, ROT, 0.3f, 0.6f });
  f.stains.push_back({ { 0.1f, -0.1f }, 0.05f, ROT, 0.3f, 0.5f });
  return f;
}

// A zombie lying dead on its back, head towards +x, arms flung out: its
// front to the sky -- shirt front, face, hands open.
Figure Fallen(const Kind& k, unsigned seed)
{
  Figure f;
  std::vector<Solid>& s = f.solids;
  const Material&     sleeve = k.longSleeves ? k.shirt : k.skin;

  // Legs: thighs, shins, shoes.
  for (const float side : { -1.0f, 1.0f }) {
    const Vector3 hip = { -0.18f, side * 0.12f, 0.1f }, knee = { -0.48f, side * 0.18f, 0.12f }, ankle = { -0.76f, side * 0.22f, 0.1f };
    s.push_back(Capsule(hip, knee, 0.1f, k.legs));
    s.push_back(Capsule(knee, ankle, 0.085f, k.legs));
    s.push_back(Ellipsoid({ -0.84f, side * 0.23f, 0.14f }, { 0.07f, 0.08f, 0.12f }, SHOE));
  }
  // The body, its front up.
  s.push_back(Ellipsoid({ 0.0f, 0.0f, 0.1f }, { 0.3f, 0.3f, 0.18f }, k.shirt));
  if (k.straps) {
    s.push_back(Ellipsoid({ 0.02f, 0.0f, 0.18f }, { 0.2f, 0.17f, 0.1f }, DENIM));
    for (const float side : { -1.0f, 1.0f }) s.push_back(Capsule({ 0.1f, side * 0.13f, 0.26f }, { 0.26f, side * 0.18f, 0.2f }, 0.035f, DENIM));
  }
  if (k.tie) {
    s.push_back(Ellipsoid({ 0.1f, 0.0f, 0.2f }, { 0.16f, 0.08f, 0.08f }, COLLAR));
    s.push_back(Capsule({ 0.22f, 0.0f, 0.28f }, { -0.1f, 0.0f, 0.26f }, 0.03f, TIE));
  }
  if (k.head == Head::Hood) s.push_back(Ellipsoid({ -0.14f, 0.0f, 0.2f }, { 0.08f, 0.14f, 0.05f }, { HOODIE.color, 0.1f, 0.1f }));
  if (!k.straps && !k.tie && k.head != Head::Hood) {
    for (int b = 0; b < 3; ++b) s.push_back(Ball({ 0.16f - 0.12f * float(b), 0.0f, 0.27f }, 0.018f, BUTTON));
  }
  if (k.ribs) {
    s.push_back(Ellipsoid({ 0.0f, 0.08f, 0.2f }, { 0.13f, 0.1f, 0.1f }, FLESH));
    for (int rib = 0; rib < 3; ++rib) {
      const float x = -0.08f + 0.06f * float(rib);
      s.push_back(Capsule({ x, 0.0f, 0.28f }, { x + 0.01f, 0.16f, 0.28f }, 0.016f, BONE));
    }
  }
  // Arms flung out, hands open.
  for (const float side : { -1.0f, 1.0f }) {
    const Vector3 shoulder = { 0.14f, side * 0.26f, 0.12f }, elbow = { 0.26f, side * 0.55f, 0.1f }, wrist = { 0.48f, side * 0.72f, 0.08f };
    s.push_back(Capsule(shoulder, elbow, 0.09f, sleeve));
    s.push_back(Capsule(elbow, wrist, 0.068f, k.longSleeves ? k.shirt : k.skin));
    const float dx = wrist.x - elbow.x, dy = wrist.y - elbow.y, len = std::sqrt(dx * dx + dy * dy);
    Hand(s, wrist, { dx / len, dy / len }, side, k.skin);
  }
  // The head, face up: skull, sockets, dead eyes, the hole of a nose, the
  // jaw fallen open.
  const bool hairFirst = k.head == Head::LongHair || k.head == Head::Hood;
  if (hairFirst) {
    // Spread out round the head on the ground: long hair, or the hood.
    s.push_back(Ellipsoid({ 0.5f, 0.0f, 0.05f }, { 0.25f, 0.27f, 0.08f }, k.head == Head::Hood ? HOODIE : k.hair));
  }
  s.push_back(Ellipsoid({ 0.45f, 0.0f, 0.12f }, { 0.19f, 0.18f, 0.18f }, k.skin));
  for (const float side : { -1.0f, 1.0f }) {
    s.push_back(Ellipsoid({ 0.44f, side * 0.18f, 0.12f }, { 0.05f, 0.035f, 0.08f }, k.skin));
    s.push_back(Ball({ 0.47f, side * 0.065f, 0.25f }, 0.045f, SOCKET));
    s.push_back(Ball({ 0.47f, side * 0.065f, 0.27f }, 0.025f, DEADEYE));
  }
  s.push_back(Capsule({ 0.52f, -0.1f, 0.28f }, { 0.52f, 0.1f, 0.28f }, 0.03f, k.skin));
  s.push_back(Ellipsoid({ 0.41f, 0.0f, 0.3f }, { 0.03f, 0.025f, 0.03f }, SOCKET));
  s.push_back(Ellipsoid({ 0.33f, 0.0f, 0.26f }, { 0.045f, 0.07f, 0.04f }, MOUTH));
  s.push_back(Capsule({ 0.36f, -0.05f, 0.285f }, { 0.36f, 0.05f, 0.285f }, 0.016f, TEETH));
  switch (k.head) {
  case Head::Cap:
    // Knocked off, lying beside the head.
    s.push_back(Ellipsoid({ 0.62f, 0.34f, 0.08f }, { 0.2f, 0.19f, 0.14f }, CAP));
    s.push_back(Ellipsoid({ 0.8f, 0.38f, 0.04f }, { 0.12f, 0.16f, 0.03f }, CAP));
    s.push_back(Ellipsoid({ 0.6f, 0.0f, 0.14f }, { 0.08f, 0.16f, 0.12f }, k.hair));
    break;
  case Head::Grey:
    s.push_back(Ellipsoid({ 0.6f, 0.0f, 0.14f }, { 0.07f, 0.16f, 0.12f }, k.hair));
    break;
  case Head::Hood:
    s.push_back(Ellipsoid({ 0.6f, 0.0f, 0.15f }, { 0.08f, 0.17f, 0.12f }, k.hair));
    break;
  case Head::Helmet:
    // Knocked off, lying beside the head.
    s.push_back(Ellipsoid({ 0.66f, -0.36f, 0.04f }, { 0.3f, 0.25f, 0.05f }, FIREHAT2));
    s.push_back(Ellipsoid({ 0.66f, -0.36f, 0.04f }, { 0.21f, 0.2f, 0.2f }, FIREHAT));
    break;
  case Head::LongHair:
  case Head::Bald:
    break;
  }
  // Blood pooled on the chest and at the mouth; dirt on the knees.
  const float spread = float(seed % 5) * 0.02f;
  f.stains.push_back({ { 0.05f + spread, -0.08f }, 0.22f, BLOOD, 0.8f });
  f.stains.push_back({ { 0.34f, 0.04f }, 0.09f, BLOOD, 0.8f });
  f.stains.push_back({ { -0.48f, 0.18f }, 0.1f, DIRT, 0.05f, 0.7f });
  f.stains.push_back({ { -0.46f, -0.2f }, 0.12f, DRIED, 0.3f, 0.7f });
  return f;
}

// ---------------------------------------------------------------- the soldier

Figure SoldierStanding()
{
  Figure f;
  std::vector<Solid>& s = f.solids;
  // The pack on his back, and the rolled blanket on top of it.
  s.push_back(Ellipsoid({ -0.34f, 0.0f, 0.2f }, { 0.2f, 0.34f, 0.3f }, PACK));
  s.push_back(Capsule({ -0.47f, -0.32f, 0.44f }, { -0.47f, 0.32f, 0.44f }, 0.1f, ROLL));
  // Body and shoulders, the straps of the pack over them.
  s.push_back(Ellipsoid({ -0.03f, 0.0f, 0.24f }, { 0.3f, 0.5f, 0.3f }, UNIFORM));
  s.push_back(Ellipsoid({ 0.0f, -0.44f, 0.3f }, { 0.17f, 0.15f, 0.2f }, UNIFORM2));
  s.push_back(Ellipsoid({ 0.0f, 0.44f, 0.3f }, { 0.17f, 0.15f, 0.2f }, UNIFORM2));
  s.push_back(Capsule({ -0.3f, -0.26f, 0.52f }, { 0.12f, -0.24f, 0.52f }, 0.035f, STRAP));
  s.push_back(Capsule({ -0.3f, 0.26f, 0.52f }, { 0.12f, 0.24f, 0.52f }, 0.035f, STRAP));
  // The right arm to the grip, the left out along the barrel.
  s.push_back(Capsule({ 0.02f, 0.44f, 0.42f }, { 0.24f, 0.3f, 0.46f }, 0.1f, UNIFORM));
  s.push_back(Capsule({ 0.24f, 0.3f, 0.46f }, { 0.3f, 0.14f, 0.52f }, 0.085f, UNIFORM));
  s.push_back(Capsule({ 0.02f, -0.44f, 0.42f }, { 0.34f, -0.3f, 0.46f }, 0.1f, UNIFORM));
  s.push_back(Capsule({ 0.34f, -0.3f, 0.46f }, { 0.58f, -0.08f, 0.52f }, 0.085f, UNIFORM));
  // The rifle: stock, body, magazine, barrel.
  s.push_back(Capsule({ -0.04f, 0.12f, 0.56f }, { 0.3f, 0.09f, 0.58f }, 0.06f, WOOD));
  s.push_back(Capsule({ 0.26f, 0.08f, 0.6f }, { 0.64f, 0.05f, 0.62f }, 0.055f, METAL));
  s.push_back(Ellipsoid({ 0.44f, 0.14f, 0.52f }, { 0.05f, 0.07f, 0.1f }, METAL));
  s.push_back(Capsule({ 0.62f, 0.05f, 0.62f }, { 0.88f, 0.038f, 0.62f }, 0.03f, METAL));
  s.push_back(Capsule({ 0.5f, 0.04f, 0.6f }, { 0.64f, 0.04f, 0.62f }, 0.045f, WOOD));
  // His hands on it.
  s.push_back(Ball({ 0.31f, 0.12f, 0.6f }, 0.075f, SKIN));
  s.push_back(Ball({ 0.6f, -0.04f, 0.6f }, 0.075f, SKIN));
  // The helmet: its brim, and the dome over it.
  s.push_back(Ellipsoid({ 0.0f, 0.0f, 0.56f }, { 0.34f, 0.33f, 0.07f }, HELMET2));
  s.push_back(Ellipsoid({ -0.01f, 0.0f, 0.56f }, { 0.29f, 0.28f, 0.3f }, HELMET));
  // The dust of the road on him.
  f.stains.push_back({ { -0.3f, 0.2f }, 0.18f, DIRT, 0.05f, 0.45f });
  f.stains.push_back({ { 0.0f, -0.42f }, 0.12f, DIRT, 0.05f, 0.4f });
  return f;
}

Figure SoldierFallen()
{
  Figure f;
  std::vector<Solid>& s = f.solids;
  // On his back: legs out behind, boots up.
  for (const float side : { -0.15f, 0.15f }) {
    s.push_back(Capsule({ -0.22f, side, 0.1f }, { -0.72f, side * 1.4f, 0.1f }, 0.12f, TROUSERS));
    s.push_back(Ellipsoid({ -0.84f, side * 1.45f, 0.14f }, { 0.08f, 0.1f, 0.16f }, BOOT));
  }
  s.push_back(Ellipsoid({ 0.0f, 0.0f, 0.1f }, { 0.34f, 0.32f, 0.2f }, UNIFORM));
  s.push_back(Capsule({ -0.1f, -0.12f, 0.28f }, { 0.22f, -0.12f, 0.28f }, 0.03f, STRAP));
  s.push_back(Capsule({ -0.1f, 0.12f, 0.28f }, { 0.22f, 0.12f, 0.28f }, 0.03f, STRAP));
  // Arms flung out.
  for (const float side : { -1.0f, 1.0f }) {
    s.push_back(Capsule({ 0.14f, side * 0.26f, 0.12f }, { 0.28f, side * 0.6f, 0.1f }, 0.09f, UNIFORM));
    s.push_back(Capsule({ 0.28f, side * 0.6f, 0.1f }, { 0.52f, side * 0.72f, 0.08f }, 0.08f, UNIFORM));
    s.push_back(Ball({ 0.58f, side * 0.74f, 0.08f }, 0.07f, SKIN));
  }
  // His head, bare, the helmet rolled away.
  s.push_back(Ball({ 0.46f, 0.0f, 0.12f }, 0.17f, SKIN));
  s.push_back(Ellipsoid({ 0.53f, 0.0f, 0.1f }, { 0.12f, 0.18f, 0.15f }, HAIR));
  s.push_back(Ellipsoid({ 0.6f, -0.52f, 0.06f }, { 0.3f, 0.29f, 0.06f }, HELMET2));
  s.push_back(Ellipsoid({ 0.6f, -0.52f, 0.06f }, { 0.25f, 0.24f, 0.24f }, HELMET));
  // The rifle, dropped beside him.
  s.push_back(Capsule({ -0.5f, 0.62f, 0.06f }, { -0.15f, 0.7f, 0.06f }, 0.06f, WOOD));
  s.push_back(Capsule({ -0.17f, 0.7f, 0.07f }, { 0.55f, 0.84f, 0.07f }, 0.045f, METAL));
  // Wounded: blood on his chest.
  f.stains.push_back({ { 0.05f, 0.06f }, 0.2f, BLOOD, 0.8f });
  return f;
}

// ---------------------------------------------------------------- drawing

// A model's pictures, on the graphics card.
struct Model {
  Texture2D colours{}, normals{}, shadow{};
};

Model Make(const Figure& figure, unsigned seed)
{
  const relief::Picture picture = relief::Build(figure.solids, figure.stains, PICTURE, seed);
  return { relief::Upload(relief::Colours(picture)), relief::Upload(relief::Normals(picture)),
           relief::Upload(relief::Shadow(picture, BLUR)) };
}

void Free(Model& model)
{
  UnloadTexture(model.colours);
  UnloadTexture(model.normals);
  UnloadTexture(model.shadow);
}

// The shader: the model's colour, lit from the game's light by which way
// its surface faces there, turned as the figure is.
constexpr const char* LIGHTING = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D normals;
uniform vec4 colDiffuse;
uniform vec2 turn;
uniform vec3 light;
out vec4 finalColor;
void main()
{
  vec4 colour = texture(texture0, fragTexCoord);
  vec4 facing = texture(normals, fragTexCoord);
  vec3 n = facing.xyz * 2.0 - 1.0;
  n = normalize(vec3(n.x * turn.x - n.y * turn.y, n.x * turn.y + n.y * turn.x, n.z));
  float diffuse  = max(dot(n, light), 0.0);
  vec3  halfway  = normalize(light + vec3(0.0, 0.0, 1.0));
  float specular = pow(max(dot(n, halfway), 0.0), 28.0) * facing.a;
  vec3  lit = colour.rgb * (0.38 + 0.82 * diffuse) + vec3(0.45 * specular);
  // As shiny as can be, it glows: a feral's eyes.
  if (facing.a > 0.97) lit = colour.rgb * 1.3 + vec3(0.25);
  finalColor = vec4(lit, colour.a) * fragColor * colDiffuse;
}
)";

// The zombies each white one may be, in the order Zombie::look picks.
const Kind* const WHITE_KINDS[] = { &FARMER, &DRIFTER, &NURSE, &BUSINESS };
constexpr int     WHITE_LOOKS   = 4;

struct Everything {
  Model  soldier, soldierFallen;
  Model  white[WHITE_LOOKS], whiteFallen[WHITE_LOOKS];
  Model  black, blackFallen;
  Model  red, redFallen;
  Shader shader{};
  int    normalsAt = 0, turnAt = 0, lightAt = 0;
};

std::optional<Everything> everything;

// Worked out the first time anything is drawn.
Everything& All()
{
  if (everything) return *everything;
  Everything& all   = everything.emplace();
  all.soldier       = Make(SoldierStanding(), 1);
  all.soldierFallen = Make(SoldierFallen(), 2);
  for (int i = 0; i < WHITE_LOOKS; ++i) {
    all.white[i]       = Make(Standing(*WHITE_KINDS[i], 10 + unsigned(i)), 10 + unsigned(i));
    all.whiteFallen[i] = Make(Fallen(*WHITE_KINDS[i], 20 + unsigned(i)), 20 + unsigned(i));
  }
  all.black       = Make(Standing(FERAL, 30), 30);
  all.blackFallen = Make(Fallen(FERAL, 31), 31);
  all.red         = Make(Standing(FIREMAN, 40), 40);
  all.redFallen   = Make(Fallen(FIREMAN, 41), 41);
  all.shader      = LoadShaderFromMemory(nullptr, LIGHTING);
  all.normalsAt   = GetShaderLocation(all.shader, "normals");
  all.turnAt      = GetShaderLocation(all.shader, "turn");
  all.lightAt     = GetShaderLocation(all.shader, "light");
  return all;
}

// A model in the middle of a square `square` pixels across, turned to face
// `facing`, `alpha` of it there: its shadow, then it, lit.
void Draw(const Model& model, Vector2 at, float square, Vector2 facing, float alpha)
{
  if (alpha <= 0.0f) return;
  Everything&     all   = All();
  const float     turn  = std::atan2(facing.y, facing.x);
  const float     size  = square * SIZE;
  const Rectangle whole = { 0.0f, 0.0f, float(model.colours.width), float(model.colours.height) };
  const Vector2   pivot = { size / 2.0f, size / 2.0f };

  DrawTexturePro(model.shadow, whole,
                 { at.x + SHADOW_FALLS.x * square, at.y + SHADOW_FALLS.y * square, size, size }, pivot, turn * RAD2DEG,
                 Fade(WHITE, SHADOW_DARK * alpha));

  BeginShaderMode(all.shader);
  const float   turned[2] = { std::cos(turn), std::sin(turn) };
  const Vector3 light     = relief::LightDirection();
  SetShaderValue(all.shader, all.turnAt, turned, SHADER_UNIFORM_VEC2);
  SetShaderValue(all.shader, all.lightAt, &light, SHADER_UNIFORM_VEC3);
  SetShaderValueTexture(all.shader, all.normalsAt, model.normals);
  DrawTexturePro(model.colours, whole, { at.x, at.y, size, size }, pivot, turn * RAD2DEG, Fade(WHITE, alpha));
  EndShaderMode();
}

// Standing, falling, or fallen: the one model fading into the other.
void DrawFalling(const Model& standing, const Model& lying, Vector2 at, float square, Vector2 facing, float fallen,
                 float alpha)
{
  const float down = std::clamp(fallen, 0.0f, 1.0f);
  if (down < 1.0f) Draw(standing, at, square, facing, alpha * (1.0f - down));
  if (down > 0.0f) Draw(lying, at, square, facing, alpha * down);
}

}  // namespace

void DrawSoldier(Vector2 at, float square, Vector2 facing, float fallen)
{
  Everything& all = All();
  DrawFalling(all.soldier, all.soldierFallen, at, square, facing, fallen, 1.0f);
}

void DrawZombie(Vector2 at, float square, Vector2 facing, const Goban::Zombie& zombie, float fallen, float fade)
{
  Everything& all = All();
  if (zombie.kind == Goban::Kind::Black) {
    DrawFalling(all.black, all.blackFallen, at, square, facing, fallen, fade);
    return;
  }
  if (zombie.kind == Goban::Kind::Red) {
    DrawFalling(all.red, all.redFallen, at, square, facing, fallen, fade);
    return;
  }
  const unsigned look = zombie.look % WHITE_LOOKS;
  DrawFalling(all.white[look], all.whiteFallen[look], at, square, facing, fallen, fade);
}

void Load()
{
  All();
}

void Unload()
{
  if (!everything) return;
  Everything& all = *everything;
  for (Model* model : { &all.soldier, &all.soldierFallen, &all.black, &all.blackFallen, &all.red, &all.redFallen }) Free(*model);
  for (int i = 0; i < WHITE_LOOKS; ++i) {
    Free(all.white[i]);
    Free(all.whiteFallen[i]);
  }
  UnloadShader(all.shader);
  everything.reset();
}

}  // namespace figures

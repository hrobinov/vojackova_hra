// Pictures of things in the round, seen from straight above: built from
// simple solids -- ellipsoids, and capsules (a ball swept along a line) --
// and worked out a pixel at a time, as a height field would be. Each pixel
// takes the solid highest there: its colour, and which way its surface
// faces. From that, a picture can be lit (Light), or its colour and the way
// its surface faces handed to a shader to light as it turns (the figures).
//
// A solid's material gives it more than a colour: a grain; a rough surface
// -- the wrinkles of cloth, the pores of skin -- which catches the light;
// a pattern woven or grown into it, a plaid, stripes, denim, veins. And over
// whatever is there, stains: blood, dirt, rot, soaked in with ragged edges.
//
// Also worked out: how much each pixel is shut in by what rises round it,
// darkening creases, as ambient light would; and the shadow it casts, a
// blurred dark silhouette.
//
// Coordinates run from -1 to 1 across the picture, x to the right, y down,
// z up out of the ground towards the viewer.

#pragma once

#include <raylib.h>

#include <vector>

namespace relief {

// The light the whole game is lit by: from the top left and high up. A unit
// long, pointing towards the light.
Vector3 LightDirection();

enum class Pattern {
  None,
  Plaid,    // a flannel check: bands of `second` both ways, darker where they cross
  Stripes,  // across, `second` bands `band` of the way across each
  Denim,    // a fine diagonal twill of `second`
  Veins,    // dark lines of `second` wandering through it, as under dead skin
  Stitches  // seams of `second`, running along
};

struct Material {
  Color   color;
  float   shine  = 0.2f;     // 0 dull to 1 glossy -- 1 glows, see Figures
  float   grain  = 0.06f;    // how much its colour varies, as cloth, skin or bark do
  float   rough  = 0.0f;     // how much its surface wrinkles, 0 smooth
  float   wrinkles = 30.0f;  // how many wrinkles a unit
  Pattern pattern = Pattern::None;
  Color   second{};          // the pattern's other colour
  float   weave  = 10.0f;    // how many of the pattern's bands a unit
  float   band   = 0.5f;     // how wide a stripe is, of its band
  bool    stains = true;     // blood and dirt show on it
};

// A stain soaked into whatever is under it: round `at`, `radius` across,
// ragged at the edge, `strength` how much it covers the colour there.
struct Stain {
  Vector2 at;
  float   radius;
  Color   color;
  float   shine    = 0.2f;
  float   strength = 1.0f;
};

struct Solid {
  enum class Shape { Ellipsoid, Capsule } shape;
  Vector3  a;               // an ellipsoid's middle, a capsule's one end
  Vector3  b{};             // a capsule's other end
  Vector3  radii{};         // an ellipsoid's radii; a capsule's radius in x
  Material material;
};

Solid Ellipsoid(Vector3 middle, Vector3 radii, const Material& material);
Solid Ball(Vector3 middle, float radius, const Material& material);
Solid Capsule(Vector3 from, Vector3 to, float radius, const Material& material);

struct Picture {
  int                size = 0;
  std::vector<Color> color;   // each pixel's colour, darkened in creases; alpha how much of it is covered
  std::vector<Color> facing;  // which way its surface faces, as a normal map (x, y, z from 0 to 255), shine in alpha
  std::vector<float> height;  // how high it is, 0 where nothing is
};

// The solids worked out as a picture `size` pixels square, stained.
Picture Build(const std::vector<Solid>& solids, const std::vector<Stain>& stains, int size, unsigned seed);
inline Picture Build(const std::vector<Solid>& solids, int size, unsigned seed)
{
  return Build(solids, {}, size, seed);
}

// The picture lit by the game's light, as it lies.
Image Light(const Picture& picture);

// Its colour and the way it faces, for a shader to light.
Image Colours(const Picture& picture);
Image Normals(const Picture& picture);

// The shadow it casts on the ground: its silhouette blurred `blur` pixels.
Image Shadow(const Picture& picture, int blur);

// Smooth noise, 0 to 1, changing about once a unit: the grain of cloth,
// skin, leaves, and the lie of the ground.
float Noise(float x, float y, unsigned seed);

// A texture of `image`, mipmapped so it stays smooth drawn small.
Texture2D Upload(Image image);

}  // namespace relief

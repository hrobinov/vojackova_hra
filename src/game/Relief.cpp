#include <game/Relief.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace relief {

namespace {

// Each pixel worked out from SAMPLES x SAMPLES points, for smooth edges.
constexpr int SAMPLES = 2;

// Creases: how far round a pixel is looked at, as a share of the picture,
// and how dark they get.
constexpr int   CREASE_SHARE = 50;
constexpr float CREASE_DEPTH = 1.6f;
constexpr float CREASE_DARKEST = 0.55f;

// Light: how bright it is where nothing faces it, and at most.
constexpr float AMBIENT = 0.42f;
constexpr float DIFFUSE = 0.78f;
constexpr float SPECULAR = 0.35f;

unsigned char Byte(float value)
{
  return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
}

// Value noise, smooth, 0 to 1: the grain of cloth, skin and leaves.
float Hash(int x, int y, unsigned seed)
{
  uint32_t h = uint32_t(x) * 374761393u + uint32_t(y) * 668265263u + seed * 2246822519u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return float((h ^ (h >> 16)) & 0xffffu) / 65535.0f;
}


Vector3 Normalized(Vector3 v)
{
  const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  return length > 0.0f ? Vector3{ v.x / length, v.y / length, v.z / length } : Vector3{ 0.0f, 0.0f, 1.0f };
}

// Where the solid is highest over (x, y), and which way it faces there; no
// height if it isn't over it at all.
bool Hit(const Solid& solid, float x, float y, float& height, Vector3& normal)
{
  if (solid.shape == Solid::Shape::Ellipsoid) {
    const float dx = x - solid.a.x, dy = y - solid.a.y;
    const float q  = (dx * dx) / (solid.radii.x * solid.radii.x) + (dy * dy) / (solid.radii.y * solid.radii.y);
    if (q >= 1.0f) return false;
    const float dz = solid.radii.z * std::sqrt(1.0f - q);
    height = solid.a.z + dz;
    normal = Normalized({ dx / (solid.radii.x * solid.radii.x), dy / (solid.radii.y * solid.radii.y),
                          dz / (solid.radii.z * solid.radii.z) });
    return true;
  }
  // A capsule: the nearest point of its line, seen from above.
  const float ex = solid.b.x - solid.a.x, ey = solid.b.y - solid.a.y;
  const float along = ex * ex + ey * ey;
  const float t  = along > 0.0f ? std::clamp(((x - solid.a.x) * ex + (y - solid.a.y) * ey) / along, 0.0f, 1.0f) : 0.0f;
  const float cx = solid.a.x + ex * t, cy = solid.a.y + ey * t, cz = solid.a.z + (solid.b.z - solid.a.z) * t;
  const float r  = solid.radii.x;
  const float d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
  if (d2 >= r * r) return false;
  const float dz = std::sqrt(r * r - d2);
  height = cz + dz;
  normal = Normalized({ x - cx, y - cy, dz });
  return true;
}

}  // namespace

float Noise(float x, float y, unsigned seed)
{
  const int   x0 = int(std::floor(x)), y0 = int(std::floor(y));
  const float fx = x - float(x0), fy = y - float(y0);
  const float sx = fx * fx * (3 - 2 * fx), sy = fy * fy * (3 - 2 * fy);
  const float top    = Hash(x0, y0, seed) + (Hash(x0 + 1, y0, seed) - Hash(x0, y0, seed)) * sx;
  const float bottom = Hash(x0, y0 + 1, seed) + (Hash(x0 + 1, y0 + 1, seed) - Hash(x0, y0 + 1, seed)) * sx;
  return top + (bottom - top) * sy;
}

Vector3 LightDirection()
{
  return Normalized({ -0.45f, -0.55f, 0.7f });
}

Solid Ellipsoid(Vector3 middle, Vector3 radii, const Material& material)
{
  return { Solid::Shape::Ellipsoid, middle, {}, radii, material };
}

Solid Ball(Vector3 middle, float radius, const Material& material)
{
  return Ellipsoid(middle, { radius, radius, radius }, material);
}

Solid Capsule(Vector3 from, Vector3 to, float radius, const Material& material)
{
  return { Solid::Shape::Capsule, from, to, { radius, radius, radius }, material };
}

namespace {

float Fraction(float v)
{
  return v - std::floor(v);
}

Color Blend(Color a, Color b, float t)
{
  t = std::clamp(t, 0.0f, 1.0f);
  return { Byte(a.r + (b.r - a.r) * t), Byte(a.g + (b.g - a.g) * t), Byte(a.b + (b.b - a.b) * t), a.a };
}

// What a material looks like at (x, y): its colour with its grain, its
// pattern and the stains over it, how shiny, and which way its surface
// faces there, roughened.
struct Surface {
  Color   color;
  float   shine;
  Vector3 normal;
};

Surface Look(const Material& m, Vector3 normal, float x, float y, const std::vector<Stain>& stains, unsigned seed)
{
  const float grain = 1.0f + m.grain * (2.0f * Noise(x * 40.0f, y * 40.0f, seed) - 1.0f)
                    + m.grain * 0.5f * (2.0f * Noise(x * 110.0f, y * 110.0f, seed + 7) - 1.0f);
  Color colour = { Byte(m.color.r * grain), Byte(m.color.g * grain), Byte(m.color.b * grain), 255 };
  float shine  = m.shine;

  // The pattern.
  const float w = m.weave;
  switch (m.pattern) {
  case Pattern::None: break;
  case Pattern::Plaid: {
    const bool across = Fraction(x * w) < 0.35f, down = Fraction(y * w) < 0.35f;
    const bool thin   = Fraction(x * w * 3.0f + 0.5f) < 0.12f || Fraction(y * w * 3.0f + 0.5f) < 0.12f;
    colour = Blend(colour, m.second, (across ? 0.5f : 0.0f) + (down ? 0.5f : 0.0f));
    if (thin) colour = Blend(colour, BLACK, 0.25f);
    break;
  }
  case Pattern::Stripes:
    if (Fraction(x * w) < m.band) colour = Blend(colour, m.second, 0.85f);
    break;
  case Pattern::Denim:
    colour = Blend(colour, m.second, 0.25f * (std::sin((x - y) * w * 40.0f) > 0.0f ? 1.0f : 0.0f) + 0.15f * Noise(x * 60, y * 60, seed + 3));
    break;
  case Pattern::Veins: {
    const float v = std::abs(2.0f * Noise(x * w, y * w, seed + 21) - 1.0f);
    const float u = std::abs(2.0f * Noise(x * w * 2.3f, y * w * 2.3f, seed + 22) - 1.0f);
    if (v < 0.06f) colour = Blend(colour, m.second, (1.0f - v / 0.06f) * 0.6f);
    if (u < 0.04f) colour = Blend(colour, m.second, (1.0f - u / 0.04f) * 0.35f);
    break;
  }
  case Pattern::Stitches:
    if (Fraction(x * w) < 0.08f && Fraction(y * w * 4.0f) < 0.5f) colour = Blend(colour, m.second, 0.8f);
    break;
  }

  // The stains, ragged at the edge, blood wet and shiny.
  if (m.stains) {
    for (const Stain& stain : stains) {
      const float dx = x - stain.at.x, dy = y - stain.at.y;
      if (std::abs(dx) > stain.radius * 1.4f || std::abs(dy) > stain.radius * 1.4f) continue;
      const float d    = std::sqrt(dx * dx + dy * dy) / stain.radius;
      const float edge = d + (Noise(x * 18.0f, y * 18.0f, seed + 51) - 0.5f) * 0.7f + (Noise(x * 55.0f, y * 55.0f, seed + 52) - 0.5f) * 0.25f;
      const float soak = std::clamp((1.0f - edge) * 3.0f, 0.0f, 1.0f) * stain.strength;
      if (soak <= 0.0f) continue;
      // Darker where it has soaked in most.
      const Color deep = { Byte(stain.color.r * (1.0f - 0.35f * soak)), Byte(stain.color.g * (1.0f - 0.35f * soak)),
                           Byte(stain.color.b * (1.0f - 0.35f * soak)), 255 };
      colour = Blend(colour, deep, soak);
      shine  = shine + (stain.shine - shine) * soak;
    }
  }

  // The surface, roughened: tipped the way its wrinkles slope.
  if (m.rough > 0.0f) {
    const float f = m.wrinkles, e = 0.25f / f;
    const auto  lie = [&](float px, float py) {
      return Noise(px * f, py * f, seed + 61) + 0.5f * Noise(px * f * 2.7f, py * f * 2.7f, seed + 62);
    };
    const float gx = (lie(x + e, y) - lie(x - e, y)) / (2.0f * e * f);
    const float gy = (lie(x, y + e) - lie(x, y - e)) / (2.0f * e * f);
    normal = Normalized({ normal.x - m.rough * gx, normal.y - m.rough * gy, normal.z });
  }
  return { colour, shine, normal };
}

}  // namespace

Picture Build(const std::vector<Solid>& solids, const std::vector<Stain>& stains, int size, unsigned seed)
{
  Picture picture;
  picture.size = size;
  picture.color.assign(size_t(size * size), Color{ 0, 0, 0, 0 });
  picture.facing.assign(size_t(size * size), Color{ 128, 128, 255, 0 });
  picture.height.assign(size_t(size * size), 0.0f);

  const float pixel = 2.0f / float(size);
  // Where each solid lies, seen from above, so a pixel only looks at those
  // over it.
  struct Bounds {
    float left, top, right, bottom;
  };
  std::vector<Bounds> bounds;
  for (const Solid& solid : solids) {
    if (solid.shape == Solid::Shape::Ellipsoid) {
      bounds.push_back({ solid.a.x - solid.radii.x, solid.a.y - solid.radii.y, solid.a.x + solid.radii.x, solid.a.y + solid.radii.y });
    } else {
      const float r = solid.radii.x;
      bounds.push_back({ std::min(solid.a.x, solid.b.x) - r, std::min(solid.a.y, solid.b.y) - r, std::max(solid.a.x, solid.b.x) + r,
                         std::max(solid.a.y, solid.b.y) + r });
    }
  }
  std::vector<const Solid*> over;
  for (int py = 0; py < size; ++py) {
    for (int px = 0; px < size; ++px) {
      float   r = 0, g = 0, b = 0, covered = 0, shine = 0, top = 0;
      Vector3 n{ 0, 0, 0 };
      const float left = -1.0f + float(px) * pixel, up = -1.0f + float(py) * pixel;
      over.clear();
      for (size_t k = 0; k < solids.size(); ++k) {
        if (bounds[k].right >= left && bounds[k].left <= left + pixel && bounds[k].bottom >= up && bounds[k].top <= up + pixel) {
          over.push_back(&solids[k]);
        }
      }
      if (over.empty()) continue;
      for (int sy = 0; sy < SAMPLES; ++sy) {
        for (int sx = 0; sx < SAMPLES; ++sx) {
          const float x = -1.0f + (float(px) + (float(sx) + 0.5f) / SAMPLES) * pixel;
          const float y = -1.0f + (float(py) + (float(sy) + 0.5f) / SAMPLES) * pixel;
          float        best = -1.0f;
          Vector3      bestNormal{};
          const Solid* bestSolid = nullptr;
          for (const Solid* solid : over) {
            float   height;
            Vector3 normal;
            if (Hit(*solid, x, y, height, normal) && height > best) {
              best       = height;
              bestNormal = normal;
              bestSolid  = solid;
            }
          }
          if (!bestSolid) continue;
          const Surface here = Look(bestSolid->material, bestNormal, x, y, stains, seed);
          r += float(here.color.r);
          g += float(here.color.g);
          b += float(here.color.b);
          n.x += here.normal.x;
          n.y += here.normal.y;
          n.z += here.normal.z;
          shine += here.shine;
          top = std::max(top, best);
          covered += 1.0f;
        }
      }
      if (covered == 0.0f) continue;
      const size_t i = size_t(py * size + px);
      picture.color[i]  = { Byte(r / covered), Byte(g / covered), Byte(b / covered),
                            Byte(255.0f * covered / float(SAMPLES * SAMPLES)) };
      n = Normalized(n);
      picture.facing[i] = { Byte((n.x * 0.5f + 0.5f) * 255.0f), Byte((n.y * 0.5f + 0.5f) * 255.0f),
                            Byte((n.z * 0.5f + 0.5f) * 255.0f), Byte(255.0f * shine / covered) };
      picture.height[i] = top;
    }
  }

  // Creases: a pixel lower than what is round it is shut in, and darker.
  const int          reach  = std::max(2, size / CREASE_SHARE);
  std::vector<Color> shaded = picture.color;
  for (int py = 0; py < size; ++py) {
    for (int px = 0; px < size; ++px) {
      const size_t i = size_t(py * size + px);
      if (picture.color[i].a == 0) continue;
      float above = 0.0f;
      int   count = 0;
      for (int k = 0; k < 8; ++k) {
        const float angle = float(k) * PI / 4.0f;
        const int   qx = px + int(std::round(std::cos(angle) * float(reach)));
        const int   qy = py + int(std::round(std::sin(angle) * float(reach)));
        if (qx < 0 || qy < 0 || qx >= size || qy >= size) continue;
        above += std::max(0.0f, picture.height[size_t(qy * size + qx)] - picture.height[i]);
        ++count;
      }
      if (count == 0) continue;
      const float shut = std::max(CREASE_DARKEST, 1.0f - CREASE_DEPTH * above / float(count));
      shaded[i] = { Byte(picture.color[i].r * shut), Byte(picture.color[i].g * shut), Byte(picture.color[i].b * shut),
                    picture.color[i].a };
    }
  }
  picture.color = std::move(shaded);
  return picture;
}

namespace {

Image ImageOf(const std::vector<Color>& pixels, int size)
{
  Image image = GenImageColor(size, size, BLANK);
  std::copy(pixels.begin(), pixels.end(), static_cast<Color*>(image.data));
  return image;
}

}  // namespace

Image Light(const Picture& picture)
{
  const Vector3      light = LightDirection();
  const Vector3      half  = Normalized({ light.x, light.y, light.z + 1.0f });
  std::vector<Color> lit(picture.color.size());
  for (size_t i = 0; i < lit.size(); ++i) {
    const Color c = picture.color[i];
    if (c.a == 0) continue;
    const Color   f = picture.facing[i];
    const Vector3 n = { float(f.r) / 127.5f - 1.0f, float(f.g) / 127.5f - 1.0f, float(f.b) / 127.5f - 1.0f };
    const float diffuse  = std::max(0.0f, n.x * light.x + n.y * light.y + n.z * light.z);
    const float specular = std::pow(std::max(0.0f, n.x * half.x + n.y * half.y + n.z * half.z), 24.0f) * float(f.a) / 255.0f;
    const float bright   = AMBIENT + DIFFUSE * diffuse;
    lit[i] = { Byte(c.r * bright + 255.0f * SPECULAR * specular), Byte(c.g * bright + 255.0f * SPECULAR * specular),
               Byte(c.b * bright + 255.0f * SPECULAR * specular), c.a };
  }
  return ImageOf(lit, picture.size);
}

Image Colours(const Picture& picture)
{
  return ImageOf(picture.color, picture.size);
}

Image Normals(const Picture& picture)
{
  return ImageOf(picture.facing, picture.size);
}

Image Shadow(const Picture& picture, int blur)
{
  // The silhouette, blurred across and then down, twice for a soft edge --
  // nothing beyond the picture's edge, so the shadow never reaches it.
  const int          size = picture.size;
  std::vector<float> a(picture.color.size()), b(a.size());
  for (size_t i = 0; i < a.size(); ++i) a[i] = float(picture.color[i].a) / 255.0f;
  for (int pass = 0; pass < 2; ++pass) {
    for (int y = 0; y < size; ++y) {
      for (int x = 0; x < size; ++x) {
        float sum = 0;
        for (int k = -blur; k <= blur; ++k) {
          if (x + k >= 0 && x + k < size) sum += a[size_t(y * size + x + k)];
        }
        b[size_t(y * size + x)] = sum / float(2 * blur + 1);
      }
    }
    for (int y = 0; y < size; ++y) {
      for (int x = 0; x < size; ++x) {
        float sum = 0;
        for (int k = -blur; k <= blur; ++k) {
          if (y + k >= 0 && y + k < size) sum += b[size_t((y + k) * size + x)];
        }
        a[size_t(y * size + x)] = sum / float(2 * blur + 1);
      }
    }
  }
  std::vector<Color> shadow(a.size());
  for (size_t i = 0; i < a.size(); ++i) shadow[i] = { 0, 0, 0, Byte(a[i] * 255.0f) };
  return ImageOf(shadow, size);
}

Texture2D Upload(Image image)
{
  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);
  GenTextureMipmaps(&texture);
  SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
  return texture;
}

}  // namespace relief

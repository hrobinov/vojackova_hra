#include <ui/Fonts.hpp>

#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ui {

// The .ttf files, embedded by tools/Embed.cpp at build time.
extern const unsigned char FONT_REGULAR_TTF[];
extern const std::size_t   FONT_REGULAR_TTF_SIZE;
extern const unsigned char FONT_SEMIBOLD_TTF[];
extern const std::size_t   FONT_SEMIBOLD_TTF_SIZE;
extern const unsigned char FONT_BOLD_TTF[];
extern const std::size_t   FONT_BOLD_TTF_SIZE;

namespace {

// Titillium Web's hhea ascender + descender (1133 + 388) over its em (1000).
constexpr float LINE_PER_EM = 1.521f;

// The .ttf of a weight.
std::pair<const unsigned char*, std::size_t> Ttf(Weight weight)
{
  switch (weight) {
  case Weight::Bold:     return { FONT_BOLD_TTF, FONT_BOLD_TTF_SIZE };
  case Weight::SemiBold: return { FONT_SEMIBOLD_TTF, FONT_SEMIBOLD_TTF_SIZE };
  default:               return { FONT_REGULAR_TTF, FONT_REGULAR_TTF_SIZE };
  }
}

// What text in a game record is written in: names, places and comments in
// the languages that use the Latin script ("žluťoučký koníček"), and the
// punctuation, dashes, quotes and currency signs typed with them.
constexpr std::pair<int, int> WANTED[] = {
  { 0x0020, 0x007E },  // ASCII
  { 0x00A0, 0x024F },  // Latin-1 Supplement, Latin Extended-A and -B
  { 0x02C6, 0x02DD },  // spacing accents
  { 0x0370, 0x03FF },  // Greek
  { 0x0400, 0x04FF },  // Cyrillic
  { 0x1E00, 0x1EFF },  // Latin Extended Additional (Vietnamese)
  { 0x2010, 0x205E },  // dashes, quotes, bullets, ellipsis, primes
  { 0x20A0, 0x20BF },  // currency signs
  { 0x2100, 0x215F },  // letterlike symbols and fractions
  { 0x2190, 0x21FF },  // arrows
  { 0x2200, 0x22FF },  // mathematical operators
};

// The codepoints of WANTED the weight's .ttf really has. Asking raylib for
// one it hasn't gives an empty glyph, drawn as nothing; left out, raylib
// draws its '?' instead. And raylib finds a character by walking the whole
// list, so it is best kept to what is there.
const std::vector<int>& Codepoints(Weight weight)
{
  static std::vector<int> have[3];
  std::vector<int>& list = have[int(weight)];
  if (!list.empty()) return list;

  std::vector<int> wanted;
  for (const auto& [from, to] : WANTED)
    for (int c = from; c <= to; ++c) wanted.push_back(c);
  const auto [data, size] = Ttf(weight);
  const int  count  = int(wanted.size());
  GlyphInfo* glyphs = LoadFontData(data, int(size), 16, wanted.data(), count, FONT_DEFAULT);
  if (!glyphs) {
    for (int c = 0x20; c <= 0x7E; ++c) list.push_back(c);
    return list;
  }
  // A glyph with no picture is one the font lacks, or a space; of those only
  // the plain space is kept, the rest would draw as nothing either way.
  for (int i = 0; i < count; ++i)
    if (glyphs[i].image.data || glyphs[i].value == ' ') list.push_back(glyphs[i].value);
  UnloadFontData(glyphs, count);
  TraceLog(LOG_INFO, "FONT: %d of %d wanted glyphs in weight %d", int(list.size()), count, int(weight));
  return list;
}

// Keyed by px * 4 + weight.
std::unordered_map<int, ::Font>& Cache()
{
  static std::unordered_map<int, ::Font> fonts;
  return fonts;
}

}  // namespace

int LineHeight(int size)
{
  return int(std::lround(float(size) * LINE_PER_EM));
}

const ::Font& FontAt(int px, Weight weight)
{
  auto& fonts = Cache();
  const int key = px * 4 + int(weight);
  if (auto it = fonts.find(key); it != fonts.end()) return it->second;

  const auto [data, size] = Ttf(weight);
  std::vector<int> codepoints = Codepoints(weight);  // raylib wants them non-const
  ::Font font = LoadFontFromMemory(".ttf", data, int(size), px, codepoints.data(), int(codepoints.size()));
  if (font.texture.id == 0) font = GetFontDefault();
  else SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return fonts.emplace(key, font).first->second;
}

float FontSpacing(int)
{
  return 0.0f;
}

void Text(const char* text, int x, int y, int px, ::Color color)
{
  DrawTextEx(FontAt(px), text, { float(x), float(y) }, float(px), FontSpacing(px), color);
}

int TextWidth(const char* text, int px)
{
  return int(std::ceil(MeasureTextEx(FontAt(px), text, float(px), FontSpacing(px)).x));
}

void UnloadFonts()
{
  const ::Font fallback = GetFontDefault();
  for (auto& [key, font] : Cache())
    if (font.texture.id != fallback.texture.id) UnloadFont(font);
  Cache().clear();
}

}  // namespace ui

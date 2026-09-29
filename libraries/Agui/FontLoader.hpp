#pragma once
#include <string>
namespace agui { class Color; class Font; }

namespace agui
{
  class FontLoader
  {
  public:
    FontLoader() = default;
    virtual ~FontLoader() = default;
    /** @return A pointer to the back end specific font or nullptr if failed and no exception was thrown.
     * @param fileName The path of the font. Must be compatible with the back end loader.
     * @param height The height of the font in pixels. */
    virtual Font* loadFont(const std::string& fileName,
                           int height,
                           int fontFlags,
                           float borderWidth,
                           const agui::Color& borderColor) = 0;
    virtual Font* loadEmptyFont() = 0;
  };
}

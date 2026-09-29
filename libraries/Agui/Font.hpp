#pragma once
#include <string>
#include <utility>
#include <memory>
#include "Agui/Color.hpp"
#include "Agui/TextEnums.hpp"
#include "Agui/Point.hpp"

namespace agui
{
  class Color;
  class FontLoader;
}

namespace agui
{
  class RichTextData
  {
  public:
    virtual ~RichTextData() {}
    int lineHeight = 0;
  };

  struct TextHighlightErrorColors
  {
    agui::Color errorColor;
    agui::Color warningColor;
    agui::Color okColor;

    static const TextHighlightErrorColors& defaults()
    {
      static TextHighlightErrorColors colors
      {
        agui::Color(1.0f, 0, 0),
        agui::Color(1.0f, 1.0f, 0),
        agui::Color(0, 1.0f, 0)
      };

      return colors;
    }

    bool operator==(const TextHighlightErrorColors& other) const = default;
    bool operator!=(const TextHighlightErrorColors& other) const = default;
  };

  class Font;
  class RichTextHandler
  {
  public:
    virtual ~RichTextHandler() = default;
    virtual int getSubstringWidth(std::string_view line, int caretPosition, const RichTextData* data, const agui::Font* defaultFont, double scale = 1);
    virtual void getTextDrawSections(std::string_view str,
                                     const Font* defaultFont,
                                     RichTextSetting richTextSetting,
                                     agui::RichTextData& data) = 0;
    virtual void getTextDrawSections(std::string_view str,
                                     const Font* defaultFont,
                                     RichTextSetting richTextSetting,
                                     std::unique_ptr<agui::RichTextData>& data) = 0;
    virtual void getTextDrawSections(std::string_view str,
                                     const Font* defaultFont,
                                     RichTextSetting richTextSetting) = 0;
    virtual int getRichTextWidth(std::string_view line, const RichTextData* data, const Font* defaultFont, double scale = 1);
    virtual int getColumnIndexFromPixelPosition(std::string_view line, const RichTextData* data, int x, const Font* defaultFont);
    virtual int getCursorMoveIndex(std::string_view fullString, const RichTextData* data, int currentIndex, TextMoveDirection direction, TextMoveType moveType);
    virtual int fixCursorPosition(const RichTextData* data, int currentIndex);
    virtual std::string_view fixNondivisibleSubstring(std::string_view substring, const RichTextData* data, const Font* defaultFont);

    struct DeleteCharsResult
    {
      std::string newText;
      int newCursorIndex;
    };
    virtual DeleteCharsResult deleteChars(std::string_view fullString, const RichTextData* data, int currentIndex, Point selection, TextMoveDirection direction, TextMoveType moveType);
  };

  /**
   * Abstract base class for all Fonts in Agui.
   *
   * Certain classes such as the ExtendedTextBox cause Agui not to be with Kerning.
   * Please disable Kerning for optimal results.
   *
   * Must implement:
   * free
   * getLineHeight
   * getHeight
   * getTextWidth
   * getPath */
  class Font
  {
    static FontLoader* loader;
  public:
    /** Should free the underlying font. */
    virtual void free() = 0;
    /** @return The font's line height which is usually the height of the highest glyph. */
    virtual int getLineHeight() const = 0;
    virtual float getAscent() const = 0;
    /** @return The glyph that parameter 'x' is inside of.
     *
     * If 'x' is before half of the glyph it returns the index of the previous glyph.
     *
     * Otherwise if 'x' is past half of the glyph it returns the index of the glyph.
     * @param str The UTF8 string to verify.
     * @param x The relative x-axis line. */
    virtual int getStringIndexFromPosition(std::string_view str, int x, RichTextSetting richTextSetting) const;
    /** @return The width of the parameter UTF-8 string. */
    virtual int getTextWidth(std::string_view text, RichTextSetting richTextSetting, double scale = 1) const = 0;
    virtual size_t getWrapIndex(std::string_view text, int width, double scale = 1) const = 0;
    virtual int getSubstringWidth(std::string_view text, const agui::RichTextData& richTextData, RichTextSetting richTextSetting = RichTextSetting::Enabled, double scale = 1) const = 0;
    virtual int getSubstringWidth(std::string_view text, int caretPosition, RichTextSetting richTextSetting, double scale = 1) const;
    virtual const char* getFontName() const;
    /** Sets the font loader for the back end. This will influence the load method. */
    static void setFontLoader(FontLoader* manager);
    /**
     * @return A pointer to the back end specific font or nullptr if failed and no exception was thrown.
     * @param fileName The path of the font. Must be compatible with the back end loader.
     * @param height The height of the font in pixels. */
    static Font* load(const std::string& fileName,
                      int height,
                      int fontFlags,
                      float borderWidth,
                      const agui::Color& borderColor);
    static Font* loadEmpty();
    virtual void reload(const std::string& fileName,
                        int height,
                        int fontFlags,
                        float borderWidth,
                        const agui::Color& borderColor) = 0;
    /** @return The path of the font. */
    virtual const std::string& getPath() const = 0;

    class DummyRichTextHandler final : public RichTextHandler
    {
    public:
      virtual void getTextDrawSections(std::string_view, const Font*, RichTextSetting, agui::RichTextData&) override {}
      virtual void getTextDrawSections(std::string_view,  const Font*, RichTextSetting, std::unique_ptr<agui::RichTextData>&) override {}
      virtual void getTextDrawSections(std::string_view, const Font*, RichTextSetting) override {}
    };

    virtual RichTextHandler* getRichTextHandler() const { static DummyRichTextHandler dummy; return &dummy; }

    Font();
    virtual ~Font();
  };
}

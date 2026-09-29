#pragma once
#include "Agui/AlignmentEnum.hpp"
#include "Agui/TextEnums.hpp"
#include "Agui/Font.hpp"
#include <limits>
#include <memory>
#include <string>
#include <vector>
namespace agui
{
  class Color;
  class Font;
  class RichTextData;
  class Graphics;
  class Rectangle;
}

namespace agui
{
  /** Allows flexible text rendering.
   * Allows rendering an area of text using alignment.
   * Also allows rendering a single line of text with an ellipsis (...). */
  class ResizableText final
  {
  public:
    enum class Result
    {
      Success,
      ExceededMaxHeight
    };

    /** Splits the text into lines that respect the maxWidth parameter and newline characters. */
    static Result multiMakeLines(const Font* font,
                                 const std::string& text,
                                 std::vector<std::string>& textRows,
                                 int maxWidth = std::numeric_limits<int>::max(),
                                 int maxHeight = std::numeric_limits<int>::max());

    static Result multiMakeLinesStrView(const Font* font,
                                        const std::string& text,
                                        std::vector<std::string_view>& textRows,
                                        int maxWidth = std::numeric_limits<int>::max(),
                                        int maxHeight = std::numeric_limits<int>::max(),
                                        RichTextData* richTextData = nullptr);

    // returns the portion of text that fits in maxWidth
    static std::string_view simpleWrapText(std::string_view text, RichTextData* richTextData, const Font* font, int maxWidth);
    static std::string_view simpleWrapTextUnsafe(std::string_view text, size_t maxLength, RichTextData* richTextData, const Font* font, int maxWidth);
    static void splitNewlinesOnly(std::string_view text, std::vector<std::string_view>& result);

    /** Ignores newline characters and adds an ellipsis if requested while respecting maxWidth. */
    static void singleMakeLines(const Font* font,
                                const std::string& text, std::string& result,
                                int maxWidth,
                                RichTextSetting richTextSetting);

    static void drawUnderline(Graphics* graphics, std::string_view text, const Font* font, const Color& color, int32_t textX, int32_t textY, RichTextSetting richTextSetting);
    static int32_t getUnderlineOffset(); // Eventually we will take into account GUI scale and font size.
    static float getUnderlineThickness();
    static void drawTextArea(Graphics* g,
                             const Font* font,
                             const Rectangle& area,
                             const Color& color,
                             const ResizableText& lines,
                             HorizontalAlign horizontalAlign,
                             VerticalAlign verticalAlign,
                             UnderlineText underlined,
                             const TextHighlightErrorColors& highlightColors);

    enum class Ellipsis { True, False };

    static void drawTextArea(Graphics* g,
                             const Font* font,
                             const Rectangle& area,
                             const Color& color,
                             const std::string& text,
                             HorizontalAlign horizontalAlign,
                             VerticalAlign verticalAlign,
                             RichTextSetting richTextSetting,
                             Ellipsis ellipsis = Ellipsis::False,
                             UnderlineText underlined = UnderlineText::False,
                             const TextHighlightErrorColors& highlightColors = TextHighlightErrorColors::defaults(),
                             HighlightedText highlighted = HighlightedText::False);

    ResizableText(const Font* font, RichTextSetting richTextSetting);
    ResizableText(const ResizableText&) = delete;
    ~ResizableText();

    void setRichTextSetting(RichTextSetting richTextSetting);
    void setString(std::string&& data, const Font* font, int maxWidth, int maxHeight);
    void setString(std::string&& data);
    void refresh(const Font* font, int maxWidth, int maxHeight);
    Result getLinebreakResult() const;

    int getMaxRowWidth() const;
    int getSubstringWidth(int row, int caretPosition) const;
    int getColumnIndexFromPixelPosition(int row, int x) const;
    int getRowWidth(int row) const;
    int getTextLength() const;

    int getCursorMoveIndex(int currentIndex, TextMoveDirection direction, TextMoveType moveType);
    RichTextHandler::DeleteCharsResult deleteChars(int currentIndex, Point selection, TextMoveDirection direction, TextMoveType moveType);

    const std::string& str() const { return this->data; }
    const std::vector<std::string_view>& lines() const;

    const RichTextData* getRichTextData() const;
    void refresh() { this->rowsStale = true; }
    RichTextSetting getRichTextSetting() const { return this->richTextSetting; }
    const Font* getFont() const;

  private:
    std::string data;
    RichTextSetting richTextSetting;
    mutable std::unique_ptr<RichTextData> richTextData;
    mutable std::vector<std::string_view> rows;
    mutable bool rowsStale = true;
    mutable Result lastResult = Result::Success;
    mutable int textLength = -1;
    int lastMaxWidth = std::numeric_limits<int>::max();
    int lastMaxHeight = std::numeric_limits<int>::max();
    const Font* lastFont = nullptr;
    int lastLineHeight = -1; // fonts can be reloaded which can change their size
  };
}

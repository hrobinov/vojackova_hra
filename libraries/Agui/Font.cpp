#include "Agui/Font.hpp"
#include "Agui/FontLoader.hpp"
#include "Agui/UTF8.hpp"
#include "Agui/Widget/TextBox.hpp"
#include <algorithm>

namespace agui
{
  int RichTextHandler::getSubstringWidth(std::string_view line, int caretPosition, const RichTextData*, const agui::Font* defaultFont, double scale)
  {
    return defaultFont->getSubstringWidth(line, caretPosition, RichTextSetting::Enabled, scale);
  }

  int RichTextHandler::getRichTextWidth(std::string_view line, const RichTextData*, const Font* defaultFont, double scale)
  {
    return defaultFont->getTextWidth(line, RichTextSetting::Enabled, scale);
  }

  int RichTextHandler::getColumnIndexFromPixelPosition(std::string_view line, const RichTextData*, int x, const Font* defaultFont)
  {
    return defaultFont->getStringIndexFromPosition(line, x, RichTextSetting::Enabled);
  }

  int RichTextHandler::getCursorMoveIndex(std::string_view fullString, const RichTextData*, int currentIndex, TextMoveDirection direction, TextMoveType moveType)
  {
    int newIndex = std::max(0, direction == TextMoveDirection::Forwards ? currentIndex + 1 : currentIndex - 1);
    if (moveType == TextMoveType::Word)
      newIndex = TextBox::findWordBoundaryInText(fullString, currentIndex, direction);

    return newIndex;
  }

  int RichTextHandler::fixCursorPosition(const RichTextData*, int currentIndex)
  {
    return currentIndex;
  }

  std::string_view RichTextHandler::fixNondivisibleSubstring(std::string_view substring, const RichTextData*, const Font*)
  {
    return substring;
  }

  RichTextHandler::DeleteCharsResult RichTextHandler::deleteChars(std::string_view fullString, const RichTextData*, int currentIndex, Point selection, TextMoveDirection direction, TextMoveType deleteType)
  {
    DeleteCharsResult result { std::string(fullString), currentIndex };

    auto removeCharsBehind = [&](int numChars)
    {
      if (result.newText.empty())
        return;

      int index = currentIndex - numChars;
      if (index < int(UTF8::length(result.newText)) && index >= 0)
      {
        UTF8::erase(result.newText, index, numChars);
        result.newCursorIndex = index;
      }
    };

    auto removeCharsInFront = [&](int numChars)
    {
      if (result.newText.empty())
        return;

      int textLen = int(UTF8::length(result.newText));
      numChars = std::min(textLen - currentIndex, numChars);
      UTF8::erase(result.newText, currentIndex, numChars);
    };

    auto deleteSelection = [&]()
    {
      UTF8::erase(result.newText, selection.x, selection.y - selection.x);
      result.newCursorIndex = selection.x;
    };

    if (selection.x != selection.y)
      deleteSelection();
    else
    {
      int charsToRemove = 1;

      if (direction == TextMoveDirection::Backwards)
      {
        if (deleteType == TextMoveType::Word)
          charsToRemove = currentIndex - TextBox::findWordBoundaryInText(result.newText, currentIndex, TextMoveDirection::Backwards);
        removeCharsBehind(charsToRemove);
      }
      else
      {
        if (deleteType == TextMoveType::Word)
          charsToRemove = TextBox::findWordBoundaryInText(result.newText, currentIndex, TextMoveDirection::Forwards) - currentIndex;
        removeCharsInFront(charsToRemove);
      }
    }

    return result;
  }

  FontLoader* Font::loader = nullptr;

  Font::Font()
  {}

  Font::~Font()
  {}

  void Font::setFontLoader(FontLoader* manager)
  {
    Font::loader = manager;
  }

  int Font::getStringIndexFromPosition(std::string_view str, int position, RichTextSetting richTextSetting) const
  {
    int lastLength = 0;
    int strLength = int(UTF8::length(str));

    for (int i = 0; i <= strLength; ++i)
    {
      int currentLength = getTextWidth(UTF8::subStr(str, 0, i), richTextSetting);

      if (currentLength == position)
      {
        return i;
      }
      else if (currentLength > position)
      {
        if (i == 0)
          return i;
        if ((lastLength + currentLength) / 2 < position)
          return i;
        else
          return i - 1;
      }
      lastLength = currentLength;
    }
    return int(UTF8::length(str)) - 1; // leave out \n
  }

  Font* Font::load(const std::string& fileName, int height, int fontFlags, float borderWidth, const agui::Color& borderColor)
  {
    return Font::loader->loadFont(fileName, height, fontFlags, borderWidth, borderColor);
  }
  Font* Font::loadEmpty()
  {
    return Font::loader->loadEmptyFont();
  }

  int Font::getSubstringWidth(std::string_view text, int caretPosition, RichTextSetting richTextSetting, double scale) const
  {
    return this->getTextWidth(UTF8::subStr(text, 0, caretPosition), richTextSetting, scale);
  }

  const char* Font::getFontName() const
  {
    return "not-defined";
  }
}

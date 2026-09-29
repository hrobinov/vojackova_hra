#include "Agui/ResizableText.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Font.hpp"
#include "Agui/UTF8.hpp"
#include <algorithm>
#include <cassert>

namespace agui
{
  void ResizableText::drawUnderline(Graphics* graphics, std::string_view text, const Font* font, const Color& color, int32_t textX, int32_t textY, RichTextSetting richTextSetting)
  {
    int32_t lineWidth = font->getTextWidth(text, richTextSetting);
    graphics->drawLine(Point(textX, textY + font->getAscent() + ResizableText::getUnderlineOffset()),
                       Point(textX + lineWidth, textY + font->getAscent() + ResizableText::getUnderlineOffset()),
                       color,
                       ResizableText::getUnderlineThickness());
  }

  int32_t ResizableText::getUnderlineOffset()
  {
    return 2;
  }

  float ResizableText::getUnderlineThickness()
  {
    return 1.0f;
  }

  void ResizableText::drawTextArea(Graphics* graphics,
                                   const Font* font,
                                   const Rectangle& area,
                                   const Color& color,
                                   const ResizableText& resizableText,
                                   HorizontalAlign horizontalAlign,
                                   VerticalAlign verticalAlign,
                                   UnderlineText underlined,
                                   const TextHighlightErrorColors& highlightColors)
  {
    int curPosX = area.getLeft();
    int curPosY = area.getTop();

    const std::vector<std::string_view>& lines = resizableText.lines();

    int lineHeight;
    if (resizableText.getRichTextData())
      lineHeight = resizableText.getRichTextData()->lineHeight;
    else
      lineHeight = font->getLineHeight();

    int lagHeight = area.height - (lineHeight * (int)lines.size());
    int verticalOffset = 0;

    if (verticalAlign == VerticalAlign::Center)
      verticalOffset = (lagHeight / 2);

    else if (verticalAlign == VerticalAlign::Bottom)
      verticalOffset = lagHeight;

    if (verticalOffset < 0)
      verticalOffset = 0;

    //std::vector<std::pair<size_t, Point>> linePositions;
    graphics->scratchLinePositions.clear();

    for (size_t i = 0; i < lines.size(); ++i)
    {
      if (horizontalAlign == HorizontalAlign::Left)
        curPosX = area.getLeft();
      else
      {
        int32_t wordWidth = 0;
        if (resizableText.getRichTextData())
          wordWidth = font->getSubstringWidth(lines[i], *resizableText.getRichTextData(), resizableText.getRichTextSetting());
        else
          wordWidth = font->getTextWidth(lines[i], resizableText.getRichTextSetting());

        int32_t lagWidth = area.width - wordWidth;
        if (horizontalAlign == HorizontalAlign::Right)
          curPosX = area.getLeft() + lagWidth - 2;
        else if (horizontalAlign == HorizontalAlign::Center)
          curPosX = area.getLeft() + (lagWidth / 2);
        else
          assert(false /* unknown horizontalAlign value */);
      }

      if (curPosY > area.getBottom() - lineHeight && i > 0)
        break;

      if (resizableText.getRichTextData())
        graphics->scratchLinePositions.emplace_back(i, Point(curPosX, curPosY + verticalOffset));
      else
      {
        graphics->drawText(Point(curPosX, curPosY + verticalOffset),
                           std::string(resizableText.lines()[i]), color, font, RichTextSetting::Disabled, HorizontalAlign::Left);
        if (bool(underlined))
          ResizableText::drawUnderline(graphics, lines[i], font, color, curPosX, curPosY + verticalOffset, resizableText.getRichTextSetting());
      }

      curPosY += lineHeight;
    }

    if (resizableText.getRichTextData())
    {
      std::vector<std::pair<size_t, Point>> linePositionsCopy;
      if (bool(underlined))
        linePositionsCopy = graphics->scratchLinePositions;

      graphics->drawTextLines(resizableText, graphics->scratchLinePositions, font, color, highlightColors);
      for (const auto& line : linePositionsCopy)
        ResizableText::drawUnderline(graphics, lines[line.first], font, color, line.second.x, line.second.y, resizableText.getRichTextSetting());
    }
  }

  void ResizableText::drawTextArea(Graphics* graphics,
                                   const Font* font,
                                   const Rectangle& area,
                                   const Color& color,
                                   const std::string& text,
                                   HorizontalAlign horizontalAlign,
                                   VerticalAlign verticalAlign,
                                   RichTextSetting richTextSetting,
                                   Ellipsis ellipsis,
                                   UnderlineText underlined,
                                   const TextHighlightErrorColors& highlightColors,
                                   HighlightedText highlighted)
  {
    int x = area.getLeft();
    int y = area.getTop();

    int lagHeight = area.height - font->getLineHeight();
    if (verticalAlign == VerticalAlign::Center)
      y += (lagHeight / 2);

    else if (verticalAlign == VerticalAlign::Bottom)
      y += lagHeight;

    if (horizontalAlign == HorizontalAlign::Left && ellipsis != Ellipsis::True)
      x = area.getLeft();
    else
    {
      int32_t wordWidth = font->getTextWidth(text, richTextSetting);
      if (wordWidth > area.width && ellipsis == Ellipsis::True)
      {
        std::string modifiedText;
        ResizableText::singleMakeLines(font, text, modifiedText, area.getWidth(), richTextSetting);
        graphics->drawText(Point(x, y), modifiedText, color, font, richTextSetting, agui::HorizontalAlign::Left, highlightColors, highlighted);
        if (bool(underlined))
          ResizableText::drawUnderline(graphics, modifiedText, font, color, x, y, richTextSetting);
        return;
      }
      int32_t lagWidth = area.width - wordWidth;

      if (horizontalAlign == HorizontalAlign::Right)
        x = area.getLeft() + lagWidth;
      else if (horizontalAlign == HorizontalAlign::Center)
        x = area.getLeft() + (lagWidth / 2);
      else // align left
        x = area.getLeft();
    }

    graphics->drawText(Point(x, y), text, color, font, richTextSetting, HorizontalAlign::Left, highlightColors, highlighted);
    if (bool(underlined))
      ResizableText::drawUnderline(graphics, text, font, color, x, y, richTextSetting);
  }

  ResizableText::Result ResizableText::multiMakeLines(const Font* font, const std::string& text, std::vector<std::string>& textRows, int maxWidth, int maxHeight)
  {
    std::vector<std::string_view> tmp;
    Result retval = multiMakeLinesStrView(font, text, tmp, maxWidth, maxHeight);
    textRows.clear();
    textRows.reserve(tmp.size());
    for (auto& str : tmp)
      textRows.emplace_back(str);

    return retval;
  }

  std::string_view ResizableText::simpleWrapText(std::string_view text, RichTextData* richTextData, const Font* font, int maxWidth)
  {
    return ResizableText::simpleWrapTextUnsafe(text, UTF8::length(text), richTextData, font, maxWidth);
  }

  std::string_view ResizableText::simpleWrapTextUnsafe(std::string_view text, size_t maxLength, RichTextData* richTextData, const Font* font, int maxWidth)
  {
    if (!richTextData)
    {
      size_t result = font->getWrapIndex(text, maxWidth);
      if (result == 0 && !text.empty())
        UTF8::bringToNextUnichar(result, text);
      return text.substr(0, result);
    }

    size_t minLength = 0;
    // maxWidth/2 characters probably won't fit on a line, so use it as top estimate for line lenght; improves perfomance for longer strings
    size_t nextGuessLength = std::min(maxLength / 2, size_t(maxWidth) / 2);

    // binary search to close by
    while (maxLength - minLength > 1)
    {
      size_t byteIndex = 0;
      for (size_t i = 0; i < nextGuessLength; i++)
        UTF8::bringToNextUnichar(byteIndex, text);

      std::string_view currentString(text.data(), byteIndex);

      int width = font->getRichTextHandler()->getRichTextWidth(currentString, richTextData, font);

      if (width > maxWidth)
        maxLength = nextGuessLength;
      else
        minLength = nextGuessLength;

      nextGuessLength = minLength + ((maxLength - minLength) / 2);
    }

    // linear search the (small) range found by binary search
    {
      size_t splitLength = minLength;

      size_t byteIndex = 0;
      for (size_t i = 0; i < splitLength; i++)
        UTF8::bringToNextUnichar(byteIndex, text);

      while (splitLength <= maxLength)
      {
        size_t lastByteIndex = byteIndex;
        size_t lastSplitLength = splitLength;

        splitLength++;
        UTF8::bringToNextUnichar(byteIndex, text);
        std::string_view currentString(text.data(), byteIndex);

        int width = font->getRichTextHandler()->getRichTextWidth(currentString, richTextData, font);

        if (width > maxWidth)
        {
          splitLength = lastSplitLength;
          byteIndex = lastByteIndex;
          break;
        }
      }

      // if requested width can't even fit the first character, return the first character anyway
      if (splitLength == 0 && !text.empty())
      {
        splitLength++;
        UTF8::bringToNextUnichar(byteIndex, text);

        std::string_view singleChar = text.substr(0, byteIndex);
        std::string_view fixedSubstr = font->getRichTextHandler()->fixNondivisibleSubstring(singleChar, richTextData, font);
        assert(singleChar.data() == fixedSubstr.data()); // the substring start shouldn't need fixing, otherwise "text" already starts in middle of valid rich-text tag
        byteIndex = fixedSubstr.length();
      }

      return text.substr(0, byteIndex);
    }
  }

  void ResizableText::splitNewlinesOnly(std::string_view text, std::vector<std::string_view>& result)
  {
      result.clear();

      size_t lastByteIndex = 0;
      size_t byteIndex = 0;

      while (true)
      {
        // currentString initialized to point into text.data() even when empty
        // so that line.data() can always be used for pointer arithmetic
        std::string_view currentString = std::string_view(text.data() + byteIndex, 0);
        std::string_view character;

        while (true)
        {
          lastByteIndex = byteIndex;
          size_t characterLength = UTF8::bringToNextUnichar(byteIndex, text);

          character = std::string_view(text.data() + lastByteIndex, characterLength);

          if (character.empty() || character == "\n")
            break;

          assert(currentString.data() + currentString.length() == character.data());
          currentString = std::string_view(currentString.data(), currentString.length() + character.length());
        }

        result.push_back(currentString);

        if (character.empty())
          return;
      }
  }

  ResizableText::Result ResizableText::multiMakeLinesStrView(const Font* font, const std::string& text, std::vector<std::string_view>& textRows, int maxWidth, int maxHeight, RichTextData* richTextData)
  {
    textRows.clear();

    std::vector<std::string_view> splitByNewline;
    ResizableText::splitNewlinesOnly(text, splitByNewline);
    int heightSoFar = 0;

    for (const auto& line : splitByNewline)
    {
      std::string_view currentlySplitting = line;
      size_t currentUTF8Length = UTF8::length(currentlySplitting);

      while (true)
      {
        // This assert is useful in finding issues but is unreasonably slow in debug when always enabled
        // assert(UTF8::length(currentlySplitting) == currentUTF8Length);
        std::string_view splitOff = currentlySplitting;
        // This doesn't need to wrap when maxWidth == ::max()
        // Even if theoretically there was enough text to wrap at that length.. no monitor in existence can ever show it.
        if (maxWidth != std::numeric_limits<int>::max())
          splitOff = ResizableText::simpleWrapTextUnsafe(currentlySplitting, currentUTF8Length, richTextData, font, maxWidth);
        size_t emptySpacesSkipped = 0;

        // align split to nearest word boundary
        if (splitOff.size() != currentlySplitting.size())
        {
          std::string_view original = splitOff;

          while(!splitOff.empty() && !::isspace(static_cast<unsigned char>(splitOff[splitOff.size() - 1])) && splitOff.back() != ']')
            splitOff = splitOff.substr(0, splitOff.size() - 1);

          size_t originalSizeAfterSpaceFound = splitOff.size();

          // remove empty space after splitting lines
          while(!splitOff.empty() && ::isspace(static_cast<unsigned char>(splitOff[splitOff.size() - 1])))
          {
            splitOff = splitOff.substr(0, splitOff.size() - 1);
            ++emptySpacesSkipped;
          }

          if (splitOff.empty())
          {
            if (emptySpacesSkipped > 0 && emptySpacesSkipped == originalSizeAfterSpaceFound)
            {
              currentUTF8Length -= UTF8::length(splitOff) + emptySpacesSkipped;
              currentlySplitting = currentlySplitting.substr(emptySpacesSkipped, currentlySplitting.size() - emptySpacesSkipped);
              continue; // only spaces in this splitoff
            }
            emptySpacesSkipped = 0;
            splitOff = original;
          }
        }

        textRows.push_back(splitOff);

        if (richTextData)
          heightSoFar += richTextData->lineHeight;
        else
          heightSoFar += font->getLineHeight();

        if (heightSoFar > maxHeight)
          return Result::ExceededMaxHeight;

        if (splitOff.size() == currentlySplitting.size())
          break;

        currentUTF8Length -= UTF8::length(splitOff) + emptySpacesSkipped;
        currentlySplitting = currentlySplitting.substr(splitOff.size() + emptySpacesSkipped, currentlySplitting.size() - splitOff.size() - emptySpacesSkipped);
      }
    }

    return Result::Success;
  }

  void ResizableText::singleMakeLines(const Font* font,
                                      const std::string& text, std::string& result,
                                      int maxWidth,
                                      RichTextSetting richTextSetting)
  {
    result.clear();

    std::string copy = text;
    std::replace(copy.begin(), copy.end(), '\n', ' ');

    if (maxWidth == std::numeric_limits<int>::max() ||
        font->getTextWidth(copy, richTextSetting) <= maxWidth)
    {
      result = std::move(copy);
      return;
    }

    maxWidth -= font->getTextWidth(std::string("..."), RichTextSetting::Disabled);

    std::unique_ptr<RichTextData> richTextData;
    if (richTextSetting != RichTextSetting::Disabled)
      font->getRichTextHandler()->getTextDrawSections(copy, font, richTextSetting, richTextData);

    std::string_view wrappedResult = simpleWrapText(copy, richTextData.get(), font, maxWidth);
    size_t deleteStart = UTF8::length(wrappedResult);
    size_t deleteEnd = UTF8::length(copy);

    if (richTextData)
      result = font->getRichTextHandler()->deleteChars(
            copy,
            richTextData.get(),
            0,
            agui::Point(int(deleteStart), int32_t(deleteEnd)),
            TextMoveDirection::Backwards, TextMoveType::SingleChar // these aren't really significant here
       ).newText;
    else
      result = font->getRichTextHandler()->agui::RichTextHandler::deleteChars(
            copy,
            richTextData.get(),
            0,
            agui::Point(int(deleteStart), int32_t(deleteEnd)),
            TextMoveDirection::Backwards, TextMoveType::SingleChar
       ).newText;

    result += "...";
  }

  ResizableText::ResizableText(const Font* font, RichTextSetting richTextSetting)
    : richTextSetting(richTextSetting)
    , lastFont(font)
  {}

  ResizableText::~ResizableText() = default;

  void ResizableText::setRichTextSetting(RichTextSetting richTextSetting)
  {
    if (this->richTextSetting == richTextSetting)
      return;

    this->richTextSetting = richTextSetting;
    if (this->richTextSetting == RichTextSetting::Disabled)
      this->richTextData.reset();
    this->rowsStale = true;
  }

  void ResizableText::setString(std::string&& data, const Font* font, int maxWidth, int maxHeight)
  {
    this->textLength = -1;
    this->data = std::move(data);
    this->rowsStale = true;

    this->lastFont = font;
    this->lastLineHeight = font->getLineHeight();
    this->lastMaxWidth = maxWidth;
    this->lastMaxHeight = maxHeight;
  }

  void ResizableText::setString(std::string&& data)
  {
    this->setString(std::move(data), this->lastFont, this->lastMaxWidth, this->lastMaxHeight);
  }

  void ResizableText::refresh(const Font* font, int maxWidth, int maxHeight)
  {
    if (font != this->lastFont ||
        font->getLineHeight() != this->lastLineHeight ||
        maxWidth != this->lastMaxWidth ||
        maxHeight != this->lastMaxHeight)
    {
      this->rowsStale = true;
      this->lastFont = font;
      this->lastLineHeight = font->getLineHeight();
      this->lastMaxWidth = maxWidth;
      this->lastMaxHeight = maxHeight;
    }
  }

  ResizableText::Result ResizableText::getLinebreakResult() const
  {
    this->lines();
    return this->lastResult;
  }

  int ResizableText::getSubstringWidth(int row, int caretPosition) const
  {
    const auto& lines = this->lines();

    if (row < 0 || row >= int(lines.size()))
      return 0;

    if (this->richTextData)
      return this->lastFont->getRichTextHandler()->getSubstringWidth(lines[row], caretPosition, this->richTextData.get(), this->lastFont);
    return this->lastFont->getSubstringWidth(lines[row], caretPosition, RichTextSetting::Disabled);
  }

  int ResizableText::getColumnIndexFromPixelPosition(int row, int x) const
  {
    const auto& lines = this->lines();

    if (row < 0 || row >= int(lines.size()))
      return 0;

    if (this->richTextData)
      return this->lastFont->getRichTextHandler()->getColumnIndexFromPixelPosition(lines[row], this->richTextData.get(), x, this->lastFont);
    else
      return this->lastFont->getStringIndexFromPosition(lines[row], x, RichTextSetting::Disabled);
  }

  int ResizableText::getRowWidth(int row) const
  {
    const auto& lines = this->lines();

    if (row < 0 || row >= int(lines.size()))
      return 0;

    if (this->richTextData)
      return this->lastFont->getRichTextHandler()->getRichTextWidth(lines[row], this->richTextData.get(), this->lastFont);
    return this->lastFont->getTextWidth(lines[row], RichTextSetting::Disabled);
  }

  int ResizableText::getMaxRowWidth() const
  {
    int result = 0;
    for (uint32_t i = 0; i < this->lines().size(); ++i)
      result = std::max(result, this->getRowWidth(i));
    return result;
  }

  int ResizableText::getTextLength() const
  {
    if (this->textLength == -1)
      this->textLength = static_cast<int>(UTF8::length(this->data));

    return this->textLength;
  }

  int ResizableText::getCursorMoveIndex(int currentIndex, TextMoveDirection direction, TextMoveType moveType)
  {
    this->lines();

    if (this->richTextData)
      return this->lastFont->getRichTextHandler()->getCursorMoveIndex(this->data, this->richTextData.get(), currentIndex, direction, moveType);
    else
      return this->lastFont->agui::Font::getRichTextHandler()->getCursorMoveIndex(this->data, nullptr, currentIndex, direction, moveType);
  }

  RichTextHandler::DeleteCharsResult ResizableText::deleteChars(int currentIndex, Point selection, TextMoveDirection direction, TextMoveType moveType)
  {
    this->lines();

    if (this->richTextData)
      return this->lastFont->getRichTextHandler()->deleteChars(this->data, this->richTextData.get(), currentIndex, selection, direction, moveType);
    return this->lastFont->agui::Font::getRichTextHandler()->deleteChars(this->data, nullptr, currentIndex, selection, direction, moveType);
  }

  const std::vector<std::string_view>& ResizableText::lines() const
  {
    if (this->rowsStale)
    {
      if (this->richTextSetting != RichTextSetting::Disabled)
        this->lastFont->getRichTextHandler()->getTextDrawSections(this->data, this->lastFont, this->richTextSetting, this->richTextData);

      this->lastResult = multiMakeLinesStrView(this->lastFont, this->data, this->rows, this->lastMaxWidth, this->lastMaxHeight, this->richTextData.get());
      this->rowsStale = false;
    }

    return this->rows;
  }

  const RichTextData* ResizableText::getRichTextData() const
  {
    this->lines(); // const function has side effects
    return this->richTextData.get();
  }

  const Font* ResizableText::getFont() const
  {
    return this->lastFont;
  }
}

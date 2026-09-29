#pragma once
#include "Agui/NamedBool.hpp"
#include <cstdint>
#include <string_view>

namespace agui
{
  enum class TextMoveDirection : uint8_t
  {
    Forwards,
    Backwards
  };

  enum class TextMoveType : uint8_t
  {
    SingleChar,
    Word
  };

  enum class RichTextSetting : uint8_t
  {
    Disabled = 0,

    ShowFontsAndColors = 1,
    ShowFollowingText = 2,
    ShowSpecialItemTags = 4, // Show eg a special blueprint tag instead of just a blueprint icon
    DetectLuaCommands = 8, // Tries to detect strings starting with /c and disable highlighting
    ShowImages = 16, // This flag is implicitly set if any other flags are set

    Enabled = ShowImages | ShowFontsAndColors,
    Highlight = ShowImages | ShowFollowingText | ShowSpecialItemTags | DetectLuaCommands,
    ChatMode = ShowImages | ShowFontsAndColors | ShowFollowingText | ShowSpecialItemTags,
    ForFollowingText = ShowImages | ShowFontsAndColors, // We use this when drawing following text. It has to have ShowFollowingText disabled, or we would cause infinite recursion
  };

  RichTextSetting parseRichTextSettingFromString(std::string_view str);
  RichTextSetting parseRichTextSettingFromInteger(uint8_t value);

  enum class UnderlineText : bool { True = true, False = false };
  using HighlightedText = NamedBool<class HighlightedTextTag>;
}

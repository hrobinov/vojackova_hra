#include <Agui/TextEnums.hpp>
#include <stdexcept>
#include <string>

namespace agui
{
  RichTextSetting parseRichTextSettingFromString(std::string_view str)
  {
    if (str == "enabled")
      return RichTextSetting::Enabled;
    if (str == "disabled")
      return RichTextSetting::Disabled;
    if (str == "highlight")
      return RichTextSetting::Highlight;

    throw std::runtime_error("unknown RichTextSetting string value: " + std::string(str));
  }

  RichTextSetting parseRichTextSettingFromInteger(uint8_t value)
  {
    switch (RichTextSetting(value))
    {
      case RichTextSetting::Disabled:
      case RichTextSetting::ShowFontsAndColors:
      case RichTextSetting::ShowFollowingText:
      case RichTextSetting::ShowSpecialItemTags:
      case RichTextSetting::DetectLuaCommands:
      case RichTextSetting::ShowImages:
      case RichTextSetting::Enabled:
      case RichTextSetting::Highlight:
      case RichTextSetting::ChatMode:
        return RichTextSetting(value);
    }

    throw std::runtime_error("unknown RichTextSetting value: " + std::to_string(value));
  }
}

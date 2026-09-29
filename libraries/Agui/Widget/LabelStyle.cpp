#include "Agui/Widget/LabelStyle.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include <Agui/StringUtil.hpp>

namespace agui
{
  std::optional<RichTextSetting> LabelStyle::overrideRichTextSetting;

  LabelStyle::LabelStyle(const LabelStyle* parent)
    : LabelStyle(nullptr, parent)
  {}

  LabelStyle::LabelStyle(Label* relatedWidget, const LabelStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    this->checkApplyOverrideRichTextSetting();
    if (relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  LabelStyle::LabelStyle(Label* relatedWidget, const LabelStyle* parent, RichTextSetting richTextSetting)
    : Style(relatedWidget, parent, false)
  {
    this->richTextSetting.reset(new RichTextSetting(richTextSetting));
    this->checkApplyOverrideRichTextSetting();
    if (relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  const agui::Color& LabelStyle::getHoveredFontColor() const
  {
    if (const agui::Color* hoveredFontColor = this->getPropertyOptional(&LabelStyle::hoveredFontColor))
      return *hoveredFontColor;
    return this->getFontColor();
  }

  const agui::Color& LabelStyle::getGameControllerHoveredFontColor() const
  {
    if (const agui::Color* hoveredFontColor = this->getPropertyOptional(&LabelStyle::gameControllerHoveredFontColor))
      return *hoveredFontColor;
    return this->getFontColor();
  }

  const Color& LabelStyle::getClickedFontColor() const
  {
    if (const agui::Color* clickedFontColor = this->getPropertyOptional(&LabelStyle::hoveredFontColor))
      return *clickedFontColor;
    return this->getHoveredFontColor();
  }

  const Color& LabelStyle::getDisabledFontColor() const
  {
    if (const agui::Color* disabledFontColor = this->getPropertyOptional(&LabelStyle::disabledFontColor))
      return *disabledFontColor;
    return this->getFontColor();
  }

  const Color& LabelStyle::getParentHoveredColor() const
  {
    if (const agui::Color* parentHoveredColor = this->getPropertyOptional(&LabelStyle::parentHoveredColor))
      return *parentHoveredColor;
    return this->getFontColor();
  }

  bool LabelStyle::isSingleLine() const
  {
    const LabelStyle* style = this;
    while ((style->labelFlags & SINGLE_LINE_DEFINED) == 0)
      style = static_cast<const LabelStyle*>(style->getParent());
    return (style->labelFlags & SINGLE_LINE_VALUE) != 0;
  }

  void LabelStyle::setSingleLine(bool value)
  {
    this->labelFlags |= SINGLE_LINE_DEFINED;
    if (value)
      this->labelFlags |= SINGLE_LINE_VALUE;
    else
    {
      this->labelFlags &= ~SINGLE_LINE_VALUE;
      this->setHorizontallySquashable(true);
    }
    if (this->relatedWidget)
    {
      this->relatedWidget->triggerResize();
      static_cast<Label*>(this->relatedWidget)->updateLabel(false);
    }
  }

  bool LabelStyle::isStrikethrough() const
  {
    const LabelStyle* style = this;
    while (style && (style->labelFlags & STRIKETHROUGH) == 0)
      style = static_cast<const LabelStyle*>(style->getParent());
    return style && ((style->labelFlags & STRIKETHROUGH) != 0);
  }

  void LabelStyle::setStrikethrough(bool value)
  {
    this->labelFlags |= STRIKETHROUGH;
    if (value)
      this->labelFlags |= STRIKETHROUGH;
    else
      this->labelFlags &= ~STRIKETHROUGH;
  }

  bool LabelStyle::isUnderlined() const
  {
    const LabelStyle* style = this;
    while (style && (style->labelFlags & UNDERLINED) == 0)
      style = static_cast<const LabelStyle*>(style->getParent());
    return style && ((style->labelFlags & UNDERLINED) != 0);
  }

  void LabelStyle::setUnderlined(bool value)
  {
    if (value)
      this->labelFlags |= UNDERLINED;
    else
      this->labelFlags &= ~UNDERLINED;
  }

  void LabelStyle::setParent(const Style* parent)
  {
    if (this->parent == parent)
      return;
    super::setParent(parent);
    if (this->relatedWidget)
    {
      this->relatedWidget->triggerResize();
      static_cast<Label*>(this->relatedWidget)->updateLabel(false);
    }
  }

  void LabelStyle::setRichTextSetting(const RichTextSetting& richTextSetting)
  {
    this->setProperty(&LabelStyle::richTextSetting, richTextSetting);
    if (this->relatedWidget)
      static_cast<Label*>(this->relatedWidget)->onRichTextSettingsChanged();
  }

  bool LabelStyle::labelFlagValueIsTheSame(const LabelStyle* other, uint8_t labelFlag) const
  {
    switch (labelFlag)
    {
      case SINGLE_LINE_DEFINED: return ((this->labelFlags & LabelStyle::SINGLE_LINE_VALUE) != 0) == ((other->labelFlags & LabelStyle::SINGLE_LINE_VALUE) != 0);
    }
    return false;
  }

  void LabelStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->font)
      this->addStyleComment(result, "font", std::string(this->font->getFontName()), this->propertyStatus(comparedWith, &LabelStyle::font));
    if (this->fontColor)
      this->addStyleComment(result, "font_color", this->fontColor->str(), this->propertyStatus(comparedWith, &LabelStyle::fontColor));
    if (this->hoveredFontColor)
      this->addStyleComment(result, "hovered_font_color", this->hoveredFontColor->str(), this->propertyStatus(comparedWith, &LabelStyle::hoveredFontColor));
    if (this->disabledFontColor)
      this->addStyleComment(result, "disabled_font_color", this->disabledFontColor->str(), this->propertyStatus(comparedWith, &LabelStyle::disabledFontColor));
    if (this->parentHoveredColor)
      this->addStyleComment(result, "parent_hovered_font_color", this->parentHoveredColor->str(), this->propertyStatus(comparedWith, &LabelStyle::parentHoveredColor));
    if (this->gameControllerHoveredFontColor)
      this->addStyleComment(result, "game_controller_hovered_font_color", this->gameControllerHoveredFontColor->str(), this->propertyStatus(comparedWith, &LabelStyle::gameControllerHoveredFontColor));
    if ((this->labelFlags & this->SINGLE_LINE_DEFINED) != 0)
      this->addStyleComment(result, "single_line", StringUtil::boolToString((this->labelFlags & this->SINGLE_LINE_VALUE) != 0), this->labelPropertyStatus(static_cast<const LabelStyle*>(comparedWith), LabelStyle::SINGLE_LINE_DEFINED));
  }

  void LabelStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->fontColor.reset();
    this->hoveredFontColor.reset();
    this->gameControllerHoveredFontColor.reset();
    this->clickedFontColor.reset();
    this->disabledFontColor.reset();
    this->parentHoveredColor.reset();
    this->richTextSetting.reset();
    this->richTextHighlightErrorColor.reset();
    this->richTextHighlightWarningColor.reset();
    this->richTextHighlightOkColor.reset();
    this->labelFlags = 0;
  }

  PropertyStatus LabelStyle::labelPropertyStatus(const LabelStyle* comparedWith, uint8_t flag) const
  {
    for (const LabelStyle* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
      if ((parent->labelFlags & flag) != 0)
      {
        if (this->labelFlagValueIsTheSame(parent, flag))
          return PropertyStatus::OverwritesTheSame;
        break;
      }

    for (;comparedWith != this; comparedWith = comparedWith->getParent())
      if ((comparedWith->labelFlags & flag) != 0)
        return PropertyStatus::Overwritten;
    return PropertyStatus::Defines;
  }

  void LabelStyle::checkApplyOverrideRichTextSetting()
  {
    if (!LabelStyle::overrideRichTextSetting)
      return;

    if (this->getRichTextSetting() != *LabelStyle::overrideRichTextSetting)
      this->richTextSetting.reset(new RichTextSetting(*LabelStyle::overrideRichTextSetting));
  }
}

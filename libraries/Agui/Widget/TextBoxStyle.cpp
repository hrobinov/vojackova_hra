#include "Agui/Widget/TextBox.hpp"
#include "Agui/Widget/TextBoxStyle.hpp"

namespace agui
{
  TextBoxStyle::TextBoxStyle(const TextBoxStyle* parent)
    : TextBoxStyle(nullptr, parent)
  {}

  TextBoxStyle::TextBoxStyle(TextBox* relatedWidget, const TextBoxStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void TextBoxStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->font)
      this->addStyleComment(result, "font", std::string(this->font->getFontName()), this->propertyStatus(comparedWith, &TextBoxStyle::font));
    if (this->fontColor)
      this->addStyleComment(result, "font_color", this->fontColor->str(), this->propertyStatus(comparedWith, &TextBoxStyle::fontColor));
    if (this->selectionBackgroundColor)
      this->addStyleComment(result, "selection_background_color", this->selectionBackgroundColor->str(), this->propertyStatus(comparedWith, &TextBoxStyle::selectionBackgroundColor));
    if (this->defaultBackground && this->parent)
      this->addStyleComment(result, "default_background", "redefined", this->propertyStatus(comparedWith, &TextBoxStyle::defaultBackground));
    if (this->activeBackground && this->parent)
      this->addStyleComment(result, "active_background", "redefined", this->propertyStatus(comparedWith, &TextBoxStyle::activeBackground));
    if (this->gameControllerHoveredBackground && this->parent)
      this->addStyleComment(result, "game_controller_hovered_background", "redefined", this->propertyStatus(comparedWith, &TextBoxStyle::gameControllerHoveredBackground));
    if (this->disabledBackground && this->parent)
      this->addStyleComment(result, "disabled_background", "redefined", this->propertyStatus(comparedWith, &TextBoxStyle::disabledBackground));
  }

  void TextBoxStyle::clear()
  {
    super::clear();
    this->font = nullptr;
    this->fontColor.reset();
    this->disabledFontColor.reset();
    this->selectionBackgroundColor.reset();
    this->defaultBackground.reset();
    this->activeBackground.reset();
    this->gameControllerHoveredBackground.reset();
    this->disabledBackground.reset();
    this->richTextSetting.reset();
    this->richTextHighlightErrorColor.reset();
    this->richTextHighlightWarningColor.reset();
    this->richTextHighlightOkColor.reset();
    this->selectedRichTextHighlightErrorColor.reset();
    this->selectedRichTextHighlightWarningColor.reset();
    this->selectedRichTextHighlightOkColor.reset();
  }

  void TextBoxStyle::setFont(const Font* font)
  {
    this->setProperty(&TextBoxStyle::font, font);
    if (this->relatedWidget)
      static_cast<TextBox*>(this->relatedWidget)->onTextSettingsChanged();
  }

  void TextBoxStyle::setRichTextSettings(const RichTextSetting& settings)
  {
    this->richTextSetting.reset(new RichTextSetting(settings));
    if (this->relatedWidget)
      static_cast<TextBox*>(this->relatedWidget)->onTextSettingsChanged();
  }

  void TextBoxStyle::setParent(const Style* parent)
  {
    super::setParent(parent);

    if (this->relatedWidget)
      static_cast<TextBox*>(this->relatedWidget)->onTextSettingsChanged();
  }
}

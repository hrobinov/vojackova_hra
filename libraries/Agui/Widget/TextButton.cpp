#include "Agui/ElementImageSet.hpp"
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Sound.hpp"
#include "Agui/Widget/TextButton.hpp"
#include <Agui/GenericTargeter.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <Agui/StringMatcher.hpp>
#include <algorithm>

namespace agui
{
  TextButton::TextButton(const ButtonStyle* parentStyle)
    : Button(parentStyle)
  {
    this->mouseButtonFilter = MouseButton::LEFT;
    this->disableFireClickOnMouseDown();
    this->resetState();
  }

  TextButton::TextButton(std::string&& text, const ButtonStyle* parentStyle)
    : TextButton(parentStyle)
  {
    this->setText(std::move(text));
  }

  void TextButton::setTextAlignment(AreaAlign alignment)
  {
    this->textAlignment = alignment;
  }

  AreaAlign TextButton::getTextAlignment() const
  {
    return this->textAlignment;
  }

  void TextButton::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    if (this->getText().empty())
      return;

    Color fontColor = this->getFontColor();
    const agui::Rectangle contentRectangle = this->getContentSizeAsRectangle();
    paintEvent.graphics()->pushClippingRect(this, contentRectangle + agui::Point(0, this->currentVerticalOffset()), true);
    int remarkSize = 0;
    if (Widget* remark = this->getRemark())
      remarkSize += remark->getWidth();
    ResizableText::drawTextArea(paintEvent.graphics(),
                                this->style.getFont(),
                                Rectangle(this->getTextOffset(),
                                          this->currentVerticalOffset(),
                                          contentRectangle.width - this->getTextOffset() - remarkSize,
                                          contentRectangle.height),
                                fontColor,
                                this->getText(),
                                this->style.getHorizontalAlign(),
                                this->style.getVerticalAlign(),
                                agui::RichTextSetting::Enabled,
                                ResizableText::Ellipsis::True,
                                UnderlineText::False,
                                TextHighlightErrorColors::defaults(),
                                HighlightedText::fromBool(this->state != ClickState::DEFAULT || this->isToggled()));
    if (this->strikethrough)
      paintEvent.graphics()->drawFilledRectangle(Rectangle(Point(this->getTextOffset(), this->getContentHeight() / 2 - 1), Dimension(this->getContentWidth(), 2)),
                                                            this->style.getStrikethroughColor());
    paintEvent.graphics()->popClippingRect();
  }

  int TextButton::getTextHeight()
  {
    return this->getText().empty() ? 0 : this->style.getFont()->getLineHeight();
  }

  int TextButton::calculateContentHeight()
  {
    return std::max(this->getContentHeight(), this->getTextHeight());
  }

  void TextButton::resizeToContents()
  {
    if (this->isKeepSize())
      return;

    if (this->getText().empty())
    {
      this->setSize(std::max(this->getHorizontalPaddings() + this->getTextOffset(), this->style.getNaturalWidth()), std::max(this->getVerticalPaddings(), this->style.getNaturalHeight()));
      return;
    }

    int width = this->style.getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled) + this->getHorizontalPaddings() + this->getTextOffset();
    if (Widget* remark = this->getRemark())
      width += remark->getWidth() + this->getRightPadding();

    this->setSize(std::max(width, this->style.getNaturalWidth()),
                  std::max(this->getVerticalPaddings() + this->style.getFont()->getLineHeight(), this->style.getNaturalHeight()));
  }

  void TextButton::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    if (Widget* remark = this->getRemark())
      remark->setLocation(agui::Point(this->getContentWidth() - remark->getWidth(), 0));
  }

  bool TextButton::shouldShowTooltip()
  {
    return this->getContentWidth() < this->style.getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled);
  }

  Widget* TextButton::getRemark()
  {
    return this->getPrivateChildCount() ? this->getPrivateChildAt(0u) : nullptr;
  }

  agui::ToolTip* TextButton::createToolTip()
  {
    if (this->shouldShowTooltip())
      return new ToolTip(this->getText(), "");
    return nullptr;
  }
  bool TextButton::genericSearch(const LowercaseString& filter)
  {
    return this->setShownBySearch(StringMatcher::matchesSearchPattern(this->getText(), filter) != StringMatcherResult::NoMatch);
  }
}

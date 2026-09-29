#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/ToolTip.hpp"
#include "Agui/Widget/Window.hpp"
#include <Agui/StringMatcher.hpp>
#include <algorithm>
#include <Agui/Gui.hpp>
#include <Agui/Input.hpp>

namespace agui
{

  agui::ToolTip* Label::createToolTip()
  {
    if (this->getContentWidth() < this->getWidestTextLineWidth() ||
        this->style.isSingleLine() && this->singleLineData != this->getText())
      return new ToolTip(this->getText(), "");
    return nullptr;
  }

  Label::StyleType Label::defaultStyle;

  Label::Label(const StyleType* parentStyle)
    : style(this, parentStyle)
    , resizableText(this->style.getFont(), this->style.getRichTextSetting())
  {
    this->shrinkInReactionToSetSize();
  }

  Label::Label(RichTextSetting richTextSetting, const StyleType* parentStyle)
    : Label(std::string(), richTextSetting, parentStyle)
  {}

  Label::Label(const std::string& text, const StyleType* parentStyle)
    : Label(std::string(text), parentStyle)
  {}

  Label::Label(const std::string& text, RichTextSetting richTextSetting, const StyleType* parentStyle)
    : Label(std::string(text), richTextSetting, parentStyle)
  {}

  Label::Label(std::string&& text, const StyleType* parentStyle)
    : style(this, parentStyle)
    , resizableText(this->style.getFont(), this->style.getRichTextSetting())
  {
    this->setText(std::move(text));
    this->shrinkInReactionToSetSize();
  }

  Label::Label(std::string&& text, const StyleType* parentStyle, RichTextSetting richTextSetting)
    : style(this, parentStyle, richTextSetting)
    , resizableText(this->style.getFont(), this->style.getRichTextSetting())
  {
    this->setText(std::move(text));
    this->shrinkInReactionToSetSize();
  }

  Label::Label(std::string&& text, RichTextSetting richTextSetting, const StyleType* parentStyle)
    : Label(std::move(text), parentStyle, richTextSetting)
  {}

  Label::~Label()
  {}

  void Label::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    this->drawText(paintEvent);
  }

  void Label::updateLabel(bool reactionToSetSize)
  {
    if (this->isSingleLine())
      ResizableText::singleMakeLines(this->style.getFont(),
                                     this->getText(),
                                     this->singleLineData,
                                     reactionToSetSize ? this->getContentWidth() : std::numeric_limits<int>::max(),
                                     this->resizableText.getRichTextSetting());
    else
      this->resizableText.refresh(this->style.getFont(),
                                  reactionToSetSize ? this->getContentWidth() : std::numeric_limits<int>::max(),
                                  std::numeric_limits<int>::max());
  }

  int Label::getRequiredHeight()
  {
    if (this->isSingleLine())
      return this->style.getFont()->getLineHeight() + this->getVerticalPaddings();

    if (const RichTextData* richTextData = this->resizableText.getRichTextData())
      return richTextData->lineHeight * this->getNumTextLines() + this->getVerticalPaddings();

    return this->style.getFont()->getLineHeight() * this->getNumTextLines() + this->getVerticalPaddings();
  }

  int Label::getWidestTextLineWidth() const
  {
    if (this->isSingleLine())
      return this->style.getFont()->getTextWidth(this->singleLineData, this->resizableText.getRichTextSetting());

    int longestTextLineWidth = 0;
    for (size_t i = 0; i < this->resizableText.lines().size(); i++)
      longestTextLineWidth = std::max(longestTextLineWidth, this->resizableText.getRowWidth(int(i)));

    return longestTextLineWidth;
  }

  void Label::setSizeToFit(std::string_view text)
  {
    if (this->style.isHorizontallyStretchable() != StretchRule::Off)
      this->style.setMinimalWidth(Label::getWidthToFit(&this->style, text));
    else
      this->style.setConstantWidth(Label::getWidthToFit(&this->style, text));
  }

  int32_t Label::getWidthToFit(const LabelStyle* labelStyle, std::string_view text)
  {
    const LabelStyle& style = labelStyle ? *labelStyle : Label::defaultStyle;
    if (style.isHorizontallyStretchable() != StretchRule::Off)
      return std::max(style.getMinimalWidth(),
                      style.getFont()->getTextWidth(text, style.getRichTextSetting()) + style.getLeftPadding() + style.getRightPadding());
    return std::max(style.getMinimalWidth(),
                    style.getFont()->getTextWidth(text, style.getRichTextSetting()) + style.getLeftPadding() + style.getRightPadding());
  }

  void Label::onRichTextSettingsChanged()
  {
    this->resizableText.setRichTextSetting(this->style.getRichTextSetting());
    this->updateLabel(false /* reaction to set size*/);
    this->triggerResize();
  }

  void Label::displaySizeChanged()
  {
    super::displaySizeChanged();
    this->resizableText.refresh();
  }

  bool Label::genericSearch(const LowercaseString& filter)
  {
    if (StringMatcher::matchesSearchPattern(this->getText(), filter) == StringMatcherResult::NoMatch)
    {
      this->hideBySearch();
      return false;
    }
    this->showBySearch();
    return true;
  }

  void Label::resizeToContents()
  {
    // specifically asking for StretchRule::On as it is the only case when I wait for stretching from outside,
    // unlike when the rule is set to StretchRule::StretchAndExpand.
    if (this->style.isHorizontallyStretchable() == StretchRule::On || this->getText().empty())
    {
      int requiredHeight = this->isSingleLine() && !this->isVerticallyStretchable() ? this->getRequiredHeight() : 0;
      Widget::setSize(0, requiredHeight, SetSizeInfo(ReactionToSetSize::False));
      if (this->isSingleLine() && this->getWidth() > 0)
        this->updateLabel(true);
      return;
    }
    int requiredWidth = this->style.getFont()->getTextWidth(this->getText(), this->resizableText.getRichTextSetting()) + this->getHorizontalPaddings();
    this->setSize(requiredWidth, this->getRequiredHeight(), SetSizeInfo(ReactionToSetSize::False));
    if (this->isSingleLine() && this->getWidth() < requiredWidth)
      this->updateLabel(true);
  }

  int Label::getRequiredWidth()
  {
    if (this->isHorizontallyStretchable())
      return 0;
    return std::max(this->getWidestTextLineWidth() + this->getHorizontalPaddings(), this->style.getMinimalWidth());
  }

  Color Label::getTextColor()
  {
    if (!this->isEnabled())
      return this->style.getDisabledFontColor();

    if (!this->mouseIsOver)
      return this->style.getFontColor();

    if (this->getGui()->input->getInputMethod() == Input::PlayerInputMethod::GameController &&
        this->style.getFontColor() == this->style.getHoveredFontColor() && //does not have a custom hover color
        this->hasToolTipCreator())
      return this->style.getGameControllerHoveredFontColor();

    return this->style.getHoveredFontColor();
  }

  void Label::drawText(const PaintEvent& paintEvent)
  {
    const agui::Rectangle contentRectangle = this->getContentSizeAsRectangle();

    // not force-clipping the label to avoid occasional clipping of the tag icons in some cases
    paintEvent.graphics()->pushClippingRect(this, contentRectangle, false);

    const Rectangle area(0, 0, contentRectangle.width, contentRectangle.height);
    Color color = this->getTextColor();

    if (this->isParentHovered())
      color = this->style.getParentHoveredColor();

    if (this->isSingleLine())
    {
      ResizableText::drawTextArea(paintEvent.graphics(),
                                  this->style.getFont(),
                                  area,
                                  color,
                                  this->singleLineData,
                                  this->style.getHorizontalAlign(),
                                  this->style.getVerticalAlign(),
                                  this->resizableText.getRichTextSetting(),
                                  ResizableText::Ellipsis::False,
                                  UnderlineText(this->style.isUnderlined()),
                                  this->getHighlightColors());
      if (this->style.isStrikethrough())
        paintEvent.graphics()->drawFilledRectangle(Rectangle(Point(0, contentRectangle.height / 2), Dimension(contentRectangle.width, 2)),
                                                             this->style.getFontColor());
    }
    else
      ResizableText::drawTextArea(paintEvent.graphics(),
                                  this->style.getFont(),
                                  area,
                                  color,
                                  this->resizableText,
                                  this->style.getHorizontalAlign(),
                                  this->style.getVerticalAlign(),
                                  UnderlineText(this->style.isUnderlined()),
                                  this->getHighlightColors());

    paintEvent.graphics()->popClippingRect();
  }

  void Label::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    if (this->getText().empty() && this->canShrink(setSizeInfo.reactionToSetSize))
      width = 0;
    Widget::setSize(width, std::min(height, this->getRequiredHeight()), setSizeInfo);
    if (width != 0)
    {
      this->updateLabel(true);
      const int widestTextLine = this->getWidestTextLineWidth();
      int desiredWidth = width;
      if (widestTextLine + this->getHorizontalPaddings() < width && this->canShrink(setSizeInfo.reactionToSetSize))
        desiredWidth = widestTextLine + this->getHorizontalPaddings();
      if (setSizeInfo.vertical != Change::Squashing && !this->isVerticallyStretchable())
        Widget::setSize(desiredWidth, this->getRequiredHeight());
      else
        Widget::setSize(desiredWidth, std::min(height, this->getRequiredHeight()));
      this->sizeBeforeStretching.height = this->getHeight();
    }
  }

  void Label::setText(const std::string& text)
  {
    if (this->getText() != text)
      this->setText(std::string(text));
  }

  void Label::setText(std::string&& text)
  {
    if (this->getText() == text)
      return;

    this->resizableText.setString(std::move(text));
    this->triggerResize();

    if (this->isHorizontallyStretchable() || this->getText().empty())
    {
      Widget::setSize(0, 0);

      // without minimal width/height, I don't really care the internal values being outdated in this case,
      // as even if the outer resize doesn't come, the 0/0 size will cause it to not even render.
      // But, if there is minimal width and minimal height, this widget will render and its internal data need to be updated.

      // I'm not calling updateLabel always just as an optimisation, as when it is resizable and will be stretched later (typical case)
      // the updateLabel will be called anyway, and since the text layouting is quite expensive operation, I don't want to do it
      // needlessly
      if (this->getWidth() > 0 && this->getHeight() > 0)
        this->updateLabel(false /* reaction to set size*/);
      return;
    }
    else
      this->updateLabel(false /* reaction to set size*/);
  }

  void Label::setSingleLine(bool singleLine)
  {
    this->style.setSingleLine(singleLine);
  }

  bool Label::isSingleLine() const
  {
    return this->style.isSingleLine();
  }

  int Label::getNumTextLines() const
  {
    if (this->isSingleLine())
      return 1;

    return int(this->resizableText.lines().size());
  }

  Window* Label::getDragTarget()
  {
    if (this->dragTarget)
      return this->dragTarget->getDragTarget();
    return nullptr;
  }

  const Window* Label::getDragTarget() const
  {
    if (this->dragTarget)
      return this->dragTarget->getDragTarget();
    return nullptr;
  }

  void Label::setDragTarget(Window* dragTarget)
  {
    this->dragTarget = dragTarget;
  }

  bool Label::mouseDown(const MouseEvent& mouseEvent)
  {
    Widget::mouseDown(mouseEvent);
    if (this->dragTarget)
      return this->dragTarget->mouseDown(mouseEvent);
    return false;
  }

  bool Label::mouseUp(const MouseEvent& mouseEvent)
  {
    if (this->dragTarget)
      return this->dragTarget->mouseUp(mouseEvent);
    return false;
  }

  bool Label::mouseDrag(const MouseEvent& mouseEvent)
  {
    Widget::mouseDrag(mouseEvent);
    if (this->dragTarget)
      return this->dragTarget->mouseDrag(mouseEvent);
    return false;
  }

  bool Label::mouseEnter(const MouseEvent& mouseEvent)
  {
    Widget::mouseEnter(mouseEvent);
    this->mouseIsOver = true;
    return true;
  }

  bool Label::mouseLeave(const MouseEvent& mouseEvent)
  {
    Widget::mouseLeave(mouseEvent);
    this->mouseIsOver = false;
    return true;
  }

  TextHighlightErrorColors Label::getHighlightColors() const
  {
    return TextHighlightErrorColors { this->style.getRichTextHighlightErrorColor(), this->style.getRichTextHighlightWarningColor(), this->style.getRichTextHighlightOkColor()};
  }
}

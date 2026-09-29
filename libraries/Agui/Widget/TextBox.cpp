#include <Agui/Widget/TextBox.hpp>
#include <Agui/Gui.hpp>
#include <Agui/PaintEvent.hpp>
#include <Agui/Graphics.hpp>
#include <Agui/UTF8.hpp>
#include <Agui/TextInputInfo.hpp>
#include <Agui/Font.hpp>
#include <Agui/KeyboardInputBlockerFactory.hpp>
#include <Agui/SystemClipboard.hpp>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <Agui/Input.hpp>

namespace agui
{
  TextBoxStyle TextBox::defaultStyle;

  TextBox::TextBox(const TextBoxStyle* parentStyle)
    : style(this, parentStyle)
    , resizableText(this->style.getFont(), this->style.getRichTextSettings())
  {
    this->applySizeRestrictionsInternal(this->style);

    this->addPrivateChild(&this->hScrollBar);
    this->addPrivateChild(&this->vScrollBar);

    this->hScrollBar.onSliderMove(this, [this]() { this->caretPixelPositionCache.reset(); });
    this->vScrollBar.onSliderMove(this, [this]()
    {
      this->caretPixelPositionCache.reset();
      this->selectionRectanglesCache.reset();
      this->rememberedVerticalPosition = this->vScrollBar.getValue();
    });

    this->setFocusable(true);
    this->setTabable(true);
    this->vScrollBar.setMouseWheelAmount(Gui::wheelScrollRate);

    this->setCaretCharPosition(Point(0, 0));
    this->lastAlign = this->getTextAlignment();
  }

  bool TextBox::isHScrollNeeded() const
  {
   if (this->getHScrollPolicy() == ScrollPolicy::Never || this->getHScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      return false;
    if (this->getHScrollPolicy() == ScrollPolicy::Always)
      return true;
    if (this->getWidestUnwrappableLineWidth() > this->getContentWidth())
      return true;
    if (this->getVScrollPolicy() != ScrollPolicy::Never &&
        (this->getLinesHeight() > this->getContentHeight() &&
         this->getWidestUnwrappableLineWidth() > (this->getContentWidth() - this->vScrollBar.getWidth())))
      return true;
    return false;
  }

  bool TextBox::isVScrollNeeded() const
  {
   if (this->getVScrollPolicy() == ScrollPolicy::Never || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      return false;
    if (this->getVScrollPolicy() == ScrollPolicy::Always)
      return true;

    if (this->getLinesHeight() > this->getContentHeight())
      return true;
    if (this->getHScrollPolicy() != ScrollPolicy::Never &&
        (this->getWidestUnwrappableLineWidth() > this->getContentWidth() &&
         this->getLinesHeight() > (this->getContentHeight() - this->hScrollBar.getHeight())))
      return true;
    return false;
  }

  void TextBox::scrollTo(int x, int y)
  {
    this->hScrollBar.setValue(x);
    this->vScrollBar.setValue(y);
  }

  void TextBox::scrollToH(int x)
  {
    this->hScrollBar.setValue(x);
  }

  void TextBox::scrollToV(int x)
  {
    this->vScrollBar.setValue(x);
  }

  int TextBox::getCurrentScrollX() const
  {
    return this->hScrollBar.getValue();
  }

  int TextBox::getCurrentScrollY() const
  {
    return this->vScrollBar.getValue();
  }

  int TextBox::getMinScrollH() const
  {
    return this->hScrollBar.getMinValue();
  }

  int TextBox::getMinScrollV() const
  {
    return this->vScrollBar.getMinValue();
  }

  int TextBox::getMaxScrollH() const
  {
    return this->hScrollBar.getMaxValue();
  }

  int TextBox::getMaxScrollV() const
  {
    return this->vScrollBar.getMaxValue();
  }

  void TextBox::checkScrollPolicy()
  {
    this->hScrollBar.setVisible(this->isHScrollNeeded());
    this->vScrollBar.setVisible(this->isVScrollNeeded());
  }

  void TextBox::resizeSBsToPolicy()
  {
    this->hScrollBar.setLocation(-this->getLeftPadding(), this->getHeight() - this->getTopPadding() - this->hScrollBar.getHeight());
    this->vScrollBar.setLocation(this->getWidth() - this->getLeftPadding() - this->vScrollBar.getWidth(), -this->getTopPadding());

    if (this->hScrollBar.isVisible() && this->vScrollBar.isVisible())
    {
      this->hScrollBar.setSize(this->vScrollBar.getAbsolutePosition().x - this->hScrollBar.getAbsolutePosition().x , this->hScrollBar.getHeight());
      this->vScrollBar.setSize(this->vScrollBar.getWidth(), this->hScrollBar.getAbsolutePosition().y - this->vScrollBar.getAbsolutePosition().y);
    }
    else if (this->hScrollBar.isVisible())
      this->hScrollBar.setSize(this->getWidth(), this->hScrollBar.getHeight());
    else if (this->vScrollBar.isVisible())
      this->vScrollBar.setSize(this->vScrollBar.getWidth(), this->getHeight());

    this->textClippingRect = Rectangle(Point(0,0),
                                       Dimension(this->getSize().width -
                                                 (this->vScrollBar.isVisible() ? this->vScrollBar.getWidth() : 0),
                                                 this->getSize().height -
                                                 (this->hScrollBar.isVisible() ? this->hScrollBar.getHeight() : 0)));
  }

  void TextBox::adjustSBRanges()
  {
    int extraH = 0;
    int extraV = 0;

    if (this->hScrollBar.isVisible())
      extraH += this->hScrollBar.getHeight();

    if (this->vScrollBar.isVisible())
      extraV += this->vScrollBar.getWidth();

    // set vertical value
    this->vScrollBar.setRangeFromPage(this->getContentHeight() - extraH, int(this->resizableText.lines().size()) * this->getLineHeight());

    // set horizontal value
    this->hScrollBar.setRangeFromPage(this->getContentWidth() + this->getWidthForTextModifier() - extraV, this->getWidestUnwrappableLineWidth());

    if (this->rememberedVerticalPosition != -1)
      this->vScrollBar.setValue(this->rememberedVerticalPosition);
  }

  void TextBox::updateScrollBars()
  {
    this->checkScrollPolicy();
    this->resizeSBsToPolicy();
    this->adjustSBRanges();
    this->caretPixelPositionCache.reset();
    if (this->isWordWrap())
      this->caretCharPositionCache.reset();
  }

  int TextBox::getLinesHeight() const
  {
    return int(this->resizableText.lines().size() * this->getLineHeight());
  }

  int TextBox::getWidestUnwrappableLineWidth() const
  {
    return this->isWordWrap() ? 0 : this->getLinePixelMetadata().widestLineWidth;
  }

  int TextBox::getWidestLineWidth() const
  {
    return this->getLinePixelMetadata().widestLineWidth;
  }

  ScrollPolicy TextBox::getHScrollPolicy() const
  {
    return this->hScrollPolicy;
  }

  ScrollPolicy TextBox::getVScrollPolicy() const
  {
    return this->vScrollPolicy;
  }

  TextBox::~TextBox(void)
  {
    this->clearKeyboardInputBlocker();
  }

  void TextBox::setHScrollPolicy(ScrollPolicy policy)
  {
    this->hScrollPolicy = policy;
    this->updateScrollBars();
  }

  void TextBox::setVScrollPolicy(ScrollPolicy policy)
  {
    this->vScrollPolicy = policy;
    this->updateScrollBars();
  }

  void TextBox::paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->paintText(paintEvent, absolutePosition, nullptr, this->getVerticalOffset());
  }

  void TextBox::paintText(const PaintEvent& paintEvent, const agui::Point& absolutePosition, const std::string* overrideText, int textStartY)
  {
    paintEvent.graphics()->pushClippingRect(this,
                                            Rectangle(this->textClippingRect.x,
                                                      this->textClippingRect.y,
                                                      this->textClippingRect.getWidth() + this->getWidthForTextModifier(),
                                                      this->textClippingRect.getHeight()),
                                            true,
                                            &absolutePosition);

    int textStartX = this->getHorizontalOffset();

    // text selection
    for (const auto& rect : this->getSelectionRectangles())
    {
      Point topLeft = rect.first;
      Point widthHeight = rect.second;
      paintEvent.graphics()->drawFilledRectangle(Rectangle(topLeft.x + textStartX, topLeft.y + textStartY, widthHeight.x, widthHeight.y),
                                                 this->style.getSelectionBackgroundColor());
    }

    if (overrideText)
      paintEvent.graphics()->drawText(Point(textStartX + this->getLineOffset(0), textStartY),
                                      *overrideText,
                                      this->getCurrentFontColor(),
                                      this->style.getFont(),
                                      RichTextSetting::Disabled,
                                      HorizontalAlign::Left);
    else
    {
      int linesSkipped = this->getVisibleLineStart();
      int maxitems = this->getVisibleLineCount();
      std::vector<std::pair<size_t, Point>>& linePositions = paintEvent.graphics()->scratchLinePositions;
      linePositions.clear();

      for (int i = linesSkipped; i <= maxitems + linesSkipped; ++i)
      {
        if (i >= (int)this->resizableText.lines().size())
          break;

        if (this->resizableText.getRichTextData())
          linePositions.emplace_back(size_t(i), Point(textStartX + this->getLineOffset(i),
                                                      textStartY + (i * this->getLineHeight())));
        else
          paintEvent.graphics()->drawText(Point(textStartX + this->getLineOffset(i),
                                                textStartY + (i * this->getLineHeight())),
                                          std::string(this->resizableText.lines()[i]).c_str(), this->getCurrentFontColor(), this->style.getFont(), RichTextSetting::Disabled, HorizontalAlign::Left);
      }

      if (this->resizableText.getRichTextData())
        paintEvent.graphics()->drawTextLines(this->resizableText, linePositions, this->style.getFont(), this->getCurrentFontColor(), this->getHighlightColors());
    }

    if (this->isFocused() && !this->isReadOnly() && this->isBlinking())
    {
      this->paintCursor(paintEvent, this->getCaretPixelPosition());
    }

    paintEvent.graphics()->popClippingRect();
  }

  void TextBox::paintCursor(const PaintEvent& paintEvent, Point topLeft)
  {
    paintEvent.graphics()->drawLine(topLeft,
                                    Point(topLeft.x, topLeft.y + this->getLineHeight()),
                                    this->getCurrentFontColor());
  }

  const TextBox::LineCharMetadata& TextBox::getLineCharMetadata() const
  {
    if (!this->lineCharMetadataCache.has_value())
    {
      LineCharMetadata& md = this->lineCharMetadataCache.emplace();
      md.lineLengths.clear();
      md.lineByteIndexes.clear();
      md.lineCharIndexes.clear();
      size_t currentByteIndex = 0;
      size_t currentCharIndex = 0;
      for (size_t i = 0; i < this->resizableText.lines().size(); ++i)
      {
        const std::string_view& line = this->resizableText.lines()[i];

        size_t bytesGap = (line.data() - this->getText().data()) - currentByteIndex;
        if (bytesGap != 0)
        {
#if DEBUG
            // Sometimes the line splitter will remove spaces/newline chars, but it shouldn't ever remove anything else
            for (size_t j = 0; j < bytesGap; j++)
              assert(::isspace(this->getText()[currentByteIndex + j]));
#endif
            currentByteIndex += bytesGap;
            currentCharIndex += bytesGap; // We know they are only whitespace, therefore 1-byte characters, so this is ok
        }

        md.lineByteIndexes.push_back(currentByteIndex);
        md.lineCharIndexes.push_back(currentCharIndex);
        size_t lineCharLength = UTF8::length(line);
        md.lineLengths.push_back(lineCharLength);
        currentCharIndex += lineCharLength;
        currentByteIndex += line.size();
      }
    }
    return *this->lineCharMetadataCache;
  }

  const TextBox::LinePixelMetadata& TextBox::getLinePixelMetadata() const
  {
    if (!this->linePixelMetadataCache.has_value())
    {
      LinePixelMetadata& md = this->linePixelMetadataCache.emplace();
      md.widestLineWidth = 0;
      md.lineWidths.clear();
      for (size_t i = 0; i < this->resizableText.lines().size(); ++i)
      {
        int lineWidth = this->getLineWidth(int(i), std::numeric_limits<int>::max());
        md.lineWidths.push_back(lineWidth);
        if (lineWidth > md.widestLineWidth)
          md.widestLineWidth = lineWidth;
      }
    }
    return *this->linePixelMetadataCache;
  }

  void TextBox::updateText()
  {
    if (this->isWordWrap())
      this->makeLinesFromWordWrap();
    else
      this->makeLinesFromNewline();

    this->boxLayoutIsValid = false;
    this->lineCharMetadataCache.reset();
    this->linePixelMetadataCache.reset();
    this->selectionRectanglesCache.reset();
  }

  void TextBox::validate()
  {
    if (!this->boxLayoutIsValid)
    {
      this->updateScrollBars();
      this->boxLayoutIsValid = true;
    }
    // Scrolling to the caret is deferred so that focusGained()
    // can indicate that it wants to do so without immediately changing the scroll position,
    // which would mess up the calculation of which position was clicked on when handling mousedown events.
    if (this->scrollToCaretDesired)
    {
      this->scrollToCaretDesired = false;
      this->scrollToCaret();
    }
  }

  void TextBox::makeLinesFromNewline()
  {
    this->resizableText.refresh(this->getFont(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
  }

  void TextBox::makeLinesFromWordWrap()
  {
    int widthForText = this->getContentWidth() + this->getWidthForTextModifier();

    int maxHeight = this->getContentHeight();
    if (this->getVScrollPolicy() == ScrollPolicy::Never)
      maxHeight = std::numeric_limits<int>::max();

    // First try fit without scrollbar
    this->resizableText.refresh(this->getFont(), widthForText, maxHeight);
    if (this->resizableText.getLinebreakResult() == ResizableText::Result::ExceededMaxHeight)
    {
      // If we need a scrollbar, then retry with less horizontal space and unlimited vertical space
      widthForText = this->getContentWidth() + this->getWidthForTextModifier() - this->vScrollBar.getWidth();
      this->resizableText.refresh(this->getFont(), widthForText, std::numeric_limits<int>::max());
    }
  }

  int TextBox::getVerticalOffset() const
  {
    if (this->vScrollBar.isVisible() || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      return -this->vScrollBar.getValue();
    return 0;
  }

  int TextBox::getHorizontalOffset() const
  {
    if (this->hScrollBar.isVisible() || this->getHScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      return -this->hScrollBar.getValue();
    return 0;
  }

  void TextBox::editText(std::string&& text)
  {
    if (this->resizableText.str() == text)
      return;
    this->setTextInternal(std::move(text));
    this->dispatchTextEdit();
  }

  void TextBox::setTextInternal(std::string&& text)
  {
    bool changed = text != this->resizableText.str();

    size_t textLengthInChars = text.size();
    size_t finalLengthInUTF8 = 0;
    if (textLengthInChars > this->getMaxLength())
    {
      size_t endIndex = this->getMaxLength();
      UTF8::bringToPrevUnichar(endIndex, text);
      text.resize(endIndex);
      finalLengthInUTF8 = UTF8::length(text);
      this->resizableText.setString(std::move(text));
    }
    else
    {
      finalLengthInUTF8 = UTF8::length(text);
      this->resizableText.setString(std::move(text));
    }

    this->clearSelection();

    if (changed)
    {
      if (this->caretCharIndex > int(finalLengthInUTF8))
        this->setCaretCharIndex(int(finalLengthInUTF8));
      if (this->dragged)
      {
        this->mouseDownIndex = 0;
        this->getGui()->forceReleaseControlWithLock();
        this->dragged = false;
      }
      this->updateText();
    }
  }

  void TextBox::setText(const std::string& text)
  {
    this->setTextInternal(std::string(text));
    if (!this->isReadOnly())
      this->setCaretCharPosition(this->charPositionFromIndex(this->getTextLength()));
  }

  void TextBox::setText(std::string&& text)
  {
    this->setTextInternal(std::move(text));
    if (!this->isReadOnly())
      this->setCaretCharPosition(this->charPositionFromIndex(this->getTextLength()));
  }

  const std::string& TextBox::getText() const
  {
    return this->resizableText.str();
  }

  int TextBox::getTextLength() const
  {
    return this->resizableText.getTextLength();
  }

  bool TextBox::mouseWheelDown(const MouseEvent& mouseEvent)
  {
    if (this->vScrollBar.isVisible() || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
    {
      this->vScrollBar.wheelScrollDown(mouseEvent.getMouseWheelChange());
      return true;
    }
    return super::mouseWheelDown(mouseEvent);
  }

  bool TextBox::mouseWheelUp(const MouseEvent& mouseEvent)
  {
    if (this->vScrollBar.isVisible() || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
    {
      this->vScrollBar.wheelScrollUp(mouseEvent.getMouseWheelChange());
      return true;
    }
    return super::mouseWheelUp(mouseEvent);
  }

  TextInputInfo TextBox::queryTextInputInfo()
  {
    Point caretPosition = this->getCaretPixelPosition();
    TextInputInfo info;
    info.textBox = this;
    info.enabled = this->isEnabled() && !this->isReadOnly();
    info.pos.x = this->getLeftPadding();
    // Use the style's top padding instead of this->getTopPadding() because the latter includes the "top border" of the textfield,
    // which we do not want.
    info.pos.y = this->getTopPadding() + caretPosition.y;
    info.width = this->getContentWidth();
    info.height = this->style.getFont()->getLineHeight();
    info.cursorOffset = caretPosition.x;
    return info;
  }

  Point TextBox::getCaretCharPosition() const
  {
    if (!this->caretCharPositionCache.has_value())
    {
      this->caretCharPositionCache.emplace(this->charPositionFromIndex(this->caretCharIndex));
    }

    return *this->caretCharPositionCache;
  }

  Point TextBox::getCaretPixelPosition() const
  {
    if (!this->caretPixelPositionCache.has_value())
    {
      Point cpos = this->getCaretCharPosition();
      int x = this->getHorizontalOffset() + this->getLineOffset(cpos.y) +
        this->getLineWidth(cpos.y, cpos.x);
      int y = this->getVerticalOffset() + (cpos.y * this->getLineHeight());
      this->caretPixelPositionCache.emplace(x, y);
    }
    return *this->caretPixelPositionCache;
  }

  void TextBox::scrollToCaret()
  {
    Point caretCharPosition = this->getCaretCharPosition();
    Point caretPixelPosition = this->getCaretPixelPosition();
    this->validate(); // Ensure scroll bars are updated because we're about to ask them about heights and stuff

    // handle row
    if (this->getHeight() != 0)
    {
      int hScrollOffset = this->hScrollBar.isVisible() ? this->hScrollBar.getHeight() : 0;

      int topEdge = 0;
      int bottomEdge = this->getContentHeight() - hScrollOffset;

      if (caretPixelPosition.y + this->getLineHeight() > bottomEdge)
      {
        int newBottom = (caretCharPosition.y + 1) * this->getLineHeight();
        this->vScrollBar.setValue(newBottom - bottomEdge);
      }

      if (caretPixelPosition.y < topEdge)
      {
        int newTop = caretCharPosition.y * this->getLineHeight();
        this->vScrollBar.setValue(newTop);
      }
    }

    // handle column
    {
      int vScrollOffset = this->vScrollBar.isVisible() ? this->vScrollBar.getWidth() : 0;

      int leftEdge = 0;
      int rightEdge = this->getContentWidth() + this->getWidthForTextModifier() - vScrollOffset;
      int lineBaseOffset = this->getLineOffset(caretCharPosition.y);
      int currentCaretPosition = this->getHorizontalOffset() + lineBaseOffset + this->getLineWidth(caretCharPosition.y, caretCharPosition.x);

      // scroll over by roughly 10 characters
      int scrollAmount = this->style.getFont()->getTextWidth(std::string("X"), RichTextSetting::Disabled) * 10;
      scrollAmount = std::min(scrollAmount, rightEdge / 3);

      if (currentCaretPosition >= rightEdge)
      {
        int newRight = lineBaseOffset + this->getLineWidth(caretCharPosition.y, caretCharPosition.x) + scrollAmount;
        this->hScrollBar.setValue(newRight - rightEdge);
      }
      else if (currentCaretPosition <= leftEdge)
      {
        int newLeft = lineBaseOffset + this->getLineWidth(caretCharPosition.y, caretCharPosition.x) - scrollAmount;
        this->hScrollBar.setValue(newLeft);
      }
      else
        this->hScrollBar.setValue(this->hScrollBar.getValue());
    }

    this->caretPixelPositionCache.reset();
    this->selectionRectanglesCache.reset();
  }

  bool TextBox::keyDown(const KeyEvent& keyEvent)
  {
    this->handleKeyboard(keyEvent);
    return true;
  }

  bool TextBox::keyRepeat(const KeyEvent& keyEvent)
  {
    this->handleKeyboard(keyEvent);
    return true;
  }

  bool TextBox::mouseDown(const MouseEvent& mouseEvent)
  {
    if (mouseEvent.getButton() == MouseButton::RIGHT)
    {
      if (this->isReadOnly())
        return false;
      this->setTextAndCallOnTextEdited(std::string());

      if (!this->isFocused())
        this->focus();
      return true;
    }

    if (mouseEvent.getButton() != MouseButton::LEFT)
      return false;
    this->dragged = false;

    // relative mouse position
    Point ppos = mouseEvent.getPosition();
    Point cpos = this->pixelToCharPosition(ppos);
    if (Gui::instance->input->getInputMethod() != Input::PlayerInputMethod::GameController)
      this->setCaretCharPosition(cpos);
    this->mouseDownIndex = this->indexFromCharPosition(cpos);

    this->clearSelection();
    return true;
  }

  bool TextBox::mouseEnter(const MouseEvent& mouseEvent)
  {
    super::mouseEnter(mouseEvent);
    this->mouseIsInside = true;
    return true;
  }

  bool TextBox::mouseLeave(const MouseEvent& mouseEvent)
  {
    super::mouseLeave(mouseEvent);
    this->mouseIsInside = false;
    return true;
  }

  Point TextBox::pixelToCharPosition(const Point& pos) const
  {
    if (this->getLineHeight() == 0)
      return Point(0, 0);

    int x = pos.x;
    int y = pos.y;

    if (this->vScrollBar.isVisible() || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      y += this->vScrollBar.getValue();
    y -= this->getTopPadding();

    int row = y / this->getLineHeight();
    int column = 0;

    if (row >= (int)this->resizableText.lines().size())
      row = (int)(this->resizableText.lines().size() - 1);

    if (row < 0)
      row = 0;

    x -= this->getLeftPadding();
    if (this->hScrollBar.isVisible() || this->getHScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      x += this->hScrollBar.getValue();
    x -= this->getLineOffset(row);

    column = this->getColumnIndexFromPixelPosition(row, x);
    return Point(column, row);
  }

  int TextBox::getColumnIndexFromPixelPosition(int row, int x) const
  {
    return this->resizableText.getColumnIndexFromPixelPosition(row, x);
  }

  void TextBox::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->boxLayoutIsValid = false;
  }

  void TextBox::flagsChanged()
  {
    Widget::flagsChanged();
    this->boxLayoutIsValid = false; // Scroll bars may need reconfiguring
  }

  void TextBox::focusGained(TabbedIn tabbedIn)
  {
    Widget::focusGained(tabbedIn);
    this->scrollToCaretDesired = true;
    this->createKeyboardInputBlocker();
    this->setBlinking(true);
    this->invalidateBlink();
    if (tabbedIn)
      this->selectAll();
  }

  void TextBox::focusLost()
  {
    Widget::focusLost();
    this->stashedSelectionIndexes = this->selectionIndexes;
    this->clearSelection();
    this->clearKeyboardInputBlocker();
  }

  int TextBox::indexFromCharPosition(const Point& cpos) const
  {
    const LineCharMetadata& lcmd = this->getLineCharMetadata();
    int row = cpos.y, col = cpos.x;
    if (row < 0)
      row = 0;
    if (row >= int(lcmd.lineCharIndexes.size()))
      row = int(lcmd.lineCharIndexes.size()) - 1;
    if (col < 0)
      col = 0;
    int lineLength = int(lcmd.lineLengths[row]);
    if (col > lineLength)
      col = lineLength;
    size_t byteIndex = lcmd.lineByteIndexes[row];
    size_t charIndex = lcmd.lineCharIndexes[row];
    for (int candidateColumn = 0; candidateColumn <= lineLength; candidateColumn++)
    {
      if (col == candidateColumn)
        return int(charIndex);

      [[maybe_unused]] size_t charLength = UTF8::bringToNextUnichar(byteIndex, this->getText());
      assert(charLength >= 1);
      ++charIndex;
    }
    return int(charIndex);
  }

  Point TextBox::charPositionFromIndex(size_t index) const
  {
    size_t row;
    const LineCharMetadata& md = this->getLineCharMetadata();

    {
      size_t rowGuessMin = 0;
      size_t rowGuessMax = md.lineCharIndexes.size();
      while (rowGuessMin < rowGuessMax - 1)
      {
        size_t lineGuessMid = rowGuessMin + (rowGuessMax - rowGuessMin) / 2;
        size_t midRowIndex = int(md.lineCharIndexes[lineGuessMid]);
        if (index >= midRowIndex)
          rowGuessMin = lineGuessMid;
        else
          rowGuessMax = lineGuessMid;
      }
      row = rowGuessMin;
    }

    size_t rowCharIndex = md.lineCharIndexes[row];
    size_t byteIndex = md.lineByteIndexes[row];
    size_t charIndex = rowCharIndex;
    while (charIndex < index)
    {
      [[maybe_unused]] size_t charLength = UTF8::bringToNextUnichar(byteIndex, this->getText());
      assert(charLength >= 1);
      ++charIndex;
    }

    return Point(int(charIndex - rowCharIndex), int(row));
  }

  void TextBox::setCaretCharPosition(const Point& cpos, bool fromResize)
  {
    this->setCaretCharIndex(this->indexFromCharPosition(cpos), fromResize);
  }

  void TextBox::setCaretCharIndex(int index, bool fromResize)
  {
    if (index < 0)
      index = 0;

    this->setBlinking(true);
    this->invalidateBlink();

    if (index == this->caretCharIndex)
      return;

    this->caretCharIndex = index;
    this->caretCharPositionCache.reset();
    this->caretPixelPositionCache.reset();

    this->arrowUpAndDownTemporaryXValue = -1;

    this->scrollToCaretDesired = true;
    if (!fromResize)
      this->rememberedVerticalPosition = this->vScrollBar.getValue();
  }

  void TextBox::onTextSettingsChanged()
  {
    this->resizableText.setRichTextSetting(this->style.getRichTextSettings());
    this->updateText();
    this->triggerResize();
  }

  bool TextBox::isWordWrap() const
  {
    return this->wordWrap;
  }

  void TextBox::setWordWrap(bool wordWrap)
  {
    if (this->wordWrap == wordWrap)
      return;
    this->wordWrap = wordWrap;
    this->updateText();
    this->updateScrollBars();
  }

  bool TextBox::pasteClipboard()
  {
    if (!this->isReadOnly())
    {
      const std::string pasteResult = SystemClipboard::paste(this->filterNewlinesInPaste);
      if (!pasteResult.empty() && this->canPasteText(pasteResult))
      {
        this->deleteSelection(false);
        this->appendText(pasteResult, true, true);
        return true;
      }
    }
    return false;
  }

  void TextBox::handleKeyboard(const KeyEvent& keyEvent)
  {
    this->textInputHandleKeyEvent(keyEvent);
  }

  void TextBox::clearKeyboardInputBlocker()
  {
    delete this->keyboardInputBlocker;
    this->keyboardInputBlocker = nullptr;
  }

  void TextBox::createKeyboardInputBlocker()
  {
    if (!this->keyboardInputBlocker && this->keyboardBlockType != KeyboardInputBlockType::None)
      this->keyboardInputBlocker = KeyboardInputBlockerFactory::createKeyboardInputBlocker(this->keyboardBlockType);
  }

  const Color& TextBox::getCurrentFontColor()
  {
    return this->isEnabled() ? this->style.getFontColor() : this->style.getDisabledFontColor();
  }

  bool TextBox::mouseDrag(const MouseEvent& mouseEvent)
  {
    Widget::mouseDrag(mouseEvent);
    if (mouseEvent.getButton() != MouseButton::LEFT || !this->isSelectable())
      return false;

    this->dragged = true;
    // relative mouse position
    Point p = mouseEvent.getPosition();

    Point cpos = this->pixelToCharPosition(p);
    this->setCaretCharPosition(cpos);

    this->setSelection(mouseDownIndex, this->indexFromCharPosition(cpos));
    return true;
  }

  void TextBox::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    // validate() also fixes child widgets.
    // This is a hack that might work due paintBackground
    // happens to be called before painting of other things.
    this->validate();
    this->chooseBackground()->base.draw(paintEvent, this->textClippingRect, absolutePosition);
  }

  void TextBox::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    auto rectangle = this->getSizeRectangle();
    if (this->vScrollBar.isVisible())
      rectangle.width -= this->vScrollBar.getWidth();
    if (this->hScrollBar.isVisible())
      rectangle.height -= this->hScrollBar.getHeight();
    if (paintEvent.graphics()->shadowView)
      this->chooseBackground()->shadow.draw(paintEvent, rectangle, absolutePosition);
  }

  const ElementImageSet* TextBox::chooseBackground() const
  {
    if (!this->isEnabled())
      return this->style.getDisabledBackground();
    if (this->isFocused() && !this->isReadOnly())
      return this->style.getActiveBackground();
    if (Gui::instance->input->getInputMethod() == Input::PlayerInputMethod::GameController)
      if (this->mouseIsInside && !this->isReadOnly())
        return this->style.getGameControllerHoveredBackground();
    return this->style.getDefaultBackground();
  }

  bool TextBox::mouseUp(const MouseEvent&)
  {
    this->dragged = false;
    return true;
  }

  int TextBox::getVisibleLineCount() const
  {
    if (this->getLineHeight() == 0)
      return 0;

    int hoffset = 0;
    if (this->hScrollBar.isVisible())
      hoffset = this->hScrollBar.getHeight();

    return ((this->getContentHeight() - hoffset) / this->getLineHeight()) + 1;
  }

  int TextBox::getVisibleLineStart() const
  {
    if (this->getLineHeight() == 0)
      return 0;
    if (this->vScrollBar.isVisible() || this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling)
      return this->vScrollBar.getValue() / this->getLineHeight();
    return 0;
  }

  const Font* TextBox::getFont() const
  {
    return this->style.getFont();
  }

  int TextBox::getLineHeight() const
  {
    if (const RichTextData* richTextData = this->resizableText.getRichTextData())
      return richTextData->lineHeight;
    else
      return this->getFont()->getLineHeight();
  }

  bool TextBox::handlesMouseWheel(bool) const
  {
    return this->hScrollBar.isVisible() || this->vScrollBar.isVisible() ||
           this->getHScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling ||
           this->getVScrollPolicy() == ScrollPolicy::DontShowButAllowScrolling;
  }

  int TextBox::getLineOffset(int line) const
  {
    int vScrollbarWidth = this->vScrollBar.isVisible() ? this->vScrollBar.getWidth() : 0;
    int textAreaWidth = this->getContentWidth() - vScrollbarWidth;
    int right = std::max(textAreaWidth, this->getWidestUnwrappableLineWidth());

    switch (this->getTextAlignment())
    {
      case HorizontalAlign::Left:
        return 0;
      case HorizontalAlign::Center:
        return (right / 2) - (this->getLineWidth(line) / 2);
      case HorizontalAlign::Right:
        return right - this->getLineWidth(line);
      default:
        throw std::runtime_error("Bad value for textAlignment");
    }
  }

  HorizontalAlign TextBox::getTextAlignment() const
  {
    return this->style.getHorizontalAlign();
  }

  void TextBox::logic(double timeElapsed)
  {
    Widget::logic(timeElapsed);
    this->processBlinkEvent(timeElapsed);

    if (this->getTextAlignment() != this->lastAlign)
    {
      this->updateText();
      this->lastAlign = this->getTextAlignment();
    }
    if (this->focusOnNextLogic)
    {
      this->focusOnNextLogic = false;
      this->focus();
    }
  }

  void TextBox::resizeToContents()
  {
    if (this->isHorizontallyStretchable())
    {
      Widget::setSize(0, 0);
      return;
    }

    // Calculate required width and height as if no line wrapping is happening.
    // If line wrapping ends up being needed setSize(...) will handle it.
    int requiredWidth = 0;
    int requiredHeight = 0;
    {
      ResizableText tempText(this->style.getFont(), this->style.getRichTextSettings());
      tempText.setString(std::string(this->getText()));
      tempText.refresh(this->getFont(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
      requiredHeight = int(this->style.getFont()->getLineHeight() * tempText.lines().size()) + this->getVerticalPaddings();
      requiredWidth = tempText.getMaxRowWidth() + this->getHorizontalMargins();
    }

    this->setSize(requiredWidth, requiredHeight);
  }

  void TextBox::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    agui::Dimension originalSize = this->getSize();
    super::setSize(width, height, setSizeInfo);

    if (this->isWordWrap())
    {
      if (originalSize != this->getSize())
        this->updateText();

      int hscroll = 0;
      if (this->isHScrollNeeded())
        hscroll = this->hScrollBar.getHeight();
      if ((setSizeInfo.horizontal == Change::Squashing || width > this->getWidth()) &&
          this->style.isHorizontallyStretchable() != StretchRule::On)
        Widget::setSize(this->getWidestLineWidth() + this->getHorizontalPaddings(),
                        this->getLinesHeight() + this->getVerticalPaddings() + hscroll);
      this->sizeBeforeStretching.height = this->getHeight();
    }
    else if (originalSize != this->getSize())
      this->updateText();

    this->updateScrollBars();
  }

  KeyboardInputBlockType TextBox::getKeyboardInputBlockType() const
  {
    return this->keyboardBlockType;
  }

  void TextBox::setKeyboardInputBlockType(KeyboardInputBlockType type)
  {
    if (this->keyboardBlockType == type)
      return;

    this->keyboardBlockType = type;
    if (this->keyboardBlockType == KeyboardInputBlockType::None)
      this->clearKeyboardInputBlocker();
    else if (this->isFocused())
      this->createKeyboardInputBlocker();
  }

  CursorProvider::CursorEnum TextBox::getEnterCursor() const
  {
    return CursorProvider::EDIT_CURSOR;
  }

  const ElementImageSet* TextBox::getBorderImageSet() const
  {
    return this->chooseBackground();
  }

  void TextBox::appendText(const std::string& text, bool atCurrentPosition /*= true*/,
                           bool repositionCaret /*= true*/)
  {
    this->deleteSelection(false);
    int index = 0;
    if (atCurrentPosition)
      index = this->getCaretCharIndex();
    else
      index = int(UTF8::length(this->getText()));

    std::string newText = this->getText();
    UTF8::insert(newText, index, text);

    int newCharIndex = 0;
    if (newText.length() > this->getMaxLength())
    {
      size_t endIndex = this->getMaxLength();
      UTF8::bringToPrevUnichar(endIndex, newText);
      newText.resize(endIndex);
      newCharIndex = int(UTF8::length(newText));
    }
    else
      newCharIndex = index + int(UTF8::length(text));

    this->editText(std::move(newText));

    if (repositionCaret)
      this->setCaretCharIndex(newCharIndex);
  }

  void TextBox::setSelectable(bool select)
  {
    this->selectable = select;
    if (!select)
      this->clearSelection();
  }

  void TextBox::selectAll()
  {
    this->setSelection(0, int(UTF8::length(this->getText())));
    this->setCaretCharIndex(this->getSelectionEnd());
  }

  void TextBox::textInputHandleKeyEvent(const KeyEvent& keyEvent)
  {
    if (this->handleHotkeys(keyEvent))
      return;

    if (keyEvent.getExtendedKey() == EXT_KEY_UP      || keyEvent.getExtendedKey() == EXT_KEY_DOWN  ||
        keyEvent.getExtendedKey() == EXT_KEY_LEFT    || keyEvent.getExtendedKey() == EXT_KEY_RIGHT ||
        keyEvent.getExtendedKey() == EXT_KEY_HOME    || keyEvent.getExtendedKey() == EXT_KEY_END   ||
        keyEvent.getExtendedKey() == EXT_KEY_PAGE_UP || keyEvent.getExtendedKey() == EXT_KEY_PAGE_DOWN)
    {
      int oldIndex = this->getCaretCharIndex();

      if (keyEvent.getExtendedKey() == EXT_KEY_HOME)
        this->setCaretCharPosition(this->findLineEdge(this->getCaretCharIndex(), TextMoveDirection::Backwards));
      else if (keyEvent.getExtendedKey() == EXT_KEY_END)
        this->setCaretCharPosition(this->findLineEdge(this->getCaretCharIndex(), TextMoveDirection::Forwards));
      else if (keyEvent.getExtendedKey() == EXT_KEY_PAGE_UP)
      {
        Point newPosition = this->charPositionFromIndex(this->getCaretCharIndex());
        newPosition.y = newPosition.y - this->getVisibleLineCount() / 2;
        this->setCaretCharPosition(newPosition);
      }
      else if (keyEvent.getExtendedKey() == EXT_KEY_PAGE_DOWN)
      {
        Point newPosition = this->charPositionFromIndex(this->getCaretCharIndex());
        newPosition.y = newPosition.y + this->getVisibleLineCount() / 2;
        this->setCaretCharPosition(newPosition);
      }
      else
      {
      #ifdef __APPLE__
        if (keyEvent.meta())
        {
          if (keyEvent.getExtendedKey() == EXT_KEY_LEFT)
            this->setCaretCharPosition(this->findLineEdge(this->getCaretCharIndex(), TextMoveDirection::Backwards));
          else if (keyEvent.getExtendedKey() == EXT_KEY_RIGHT)
            this->setCaretCharPosition(this->findLineEdge(this->getCaretCharIndex(), TextMoveDirection::Forwards));
          else if (keyEvent.getExtendedKey() == EXT_KEY_UP)
            this->setCaretCharPosition({ 0, 0 });
          else if (keyEvent.getExtendedKey() == EXT_KEY_DOWN)
            this->setCaretCharIndex(static_cast<int>(UTF8::length(this->getText()) - 1));
        }
        else
          this->handleArrowKey(keyEvent);
      #else
        this->handleArrowKey(keyEvent);
      #endif
      }

      if (keyEvent.shift())
      {
        int newIndex = this->getCaretCharIndex();
        if (oldIndex == newIndex)
          return;
        if (this->isSelectionEmpty())
          this->setSelection(oldIndex, newIndex);
        else
        {
          if (oldIndex == this->getSelectionEnd())
            this->setSelection(this->getSelectionStart(), newIndex);
          else
            this->setSelection(this->getSelectionEnd(), newIndex);
        }
      }
      else
        this->clearSelection();
    }
    else if (!this->isReadOnly())
    {
      if (keyEvent.getKey() == KEY_BACKSPACE || keyEvent.getKey() == KEY_DELETE)
      {
        RichTextHandler::DeleteCharsResult result = this->resizableText.deleteChars(this->getCaretCharIndex(),
                                                                                    this->selectionIndexes,
                                                                                    keyEvent.getKey() == KEY_BACKSPACE ? TextMoveDirection::Backwards : TextMoveDirection::Forwards,
                                                                                    keyEvent.optionOrControl() ? TextMoveType::Word : TextMoveType::SingleChar);
        this->editText(std::move(result.newText));
        const int fixedIndex = this->getFont()->getRichTextHandler()->fixCursorPosition(this->resizableText.getRichTextData(), result.newCursorIndex);
        this->setCaretCharIndex(fixedIndex);
      }
      else if (keyEvent.getKey() == KEY_ENTER)
      {
        if (keyEvent.shift() || !this->confirmOnEnter)
        {
          if (!this->isSelectionEmpty())
            this->deleteSelection(false);

          this->addToNextCharacter('\n');
        }
        else
          this->dispatchConfirm(keyEvent);
      }
      else if (keyEvent.getKey() == KEY_TAB && keyEvent.metaOrControl())
      {
        if (!this->isSelectionEmpty())
          this->deleteSelection(false);

        for (size_t i = 0; i < 4; i++)
          this->addToNextCharacter(' ');
      }
      else if (keyEvent.getUnichar() >= ' ')
      {
        if (!this->isSelectionEmpty())
          this->deleteSelection(false);
        this->addToNextCharacter(keyEvent.getUnichar());
      }
    }
  }

  void TextBox::displaySizeChanged()
  {
    super::displaySizeChanged();
    this->resizableText.refresh();
  }

  Point TextBox::findLineEdge(int currentPosition, TextMoveDirection direction)
  {
    Point position = this->charPositionFromIndex(currentPosition);
    if (direction == TextMoveDirection::Backwards)
      position.x = 0;
    else
      position.x = this->resizableText.lines().empty() ? 0 : int(UTF8::length(this->resizableText.lines()[position.y]));

    return position;
  }

  void TextBox::handleArrowKey(const KeyEvent& keyEvent)
  {
    int oldIndex = this->getCaretCharIndex();
    Point oldPosition = this->charPositionFromIndex(oldIndex);

    if (keyEvent.getExtendedKey() == EXT_KEY_LEFT || keyEvent.getExtendedKey() == EXT_KEY_RIGHT)
    {
      TextMoveDirection direction = keyEvent.getExtendedKey() == EXT_KEY_RIGHT ? TextMoveDirection::Forwards : TextMoveDirection::Backwards;
      TextMoveType type = keyEvent.optionOrControl() ? TextMoveType::Word : TextMoveType::SingleChar;

      int newIndex = this->resizableText.getCursorMoveIndex(oldIndex, direction, type);
      this->setCaretCharIndex(newIndex);
    }
    else if (keyEvent.getExtendedKey() == EXT_KEY_DOWN || keyEvent.getExtendedKey() == EXT_KEY_UP)
    {
      int newY = keyEvent.getExtendedKey() == EXT_KEY_DOWN ? oldPosition.y + 1 : oldPosition.y - 1;

      // Try to find character on the new line with the closest x coordinate as possible to
      // the one we're moving from.
      if (newY >= 0 && newY < int(this->resizableText.lines().size()))
      {
        // Saving this temporary allows us to keep the same "target x position" for a whole string of
        // arrows up and down. This means we can eg. arrow up to a shorter line, then up again to a long one
        // and still keep the position we had at the beginning instead of moving to the position we were
        // at in the short one.
        // The temporary is cleared whenever the cursor moves for any reason other than arrow up or down.
        int targetXPosition = this->arrowUpAndDownTemporaryXValue;

        if (targetXPosition == -1)
        {
          targetXPosition = this->getLineWidth(oldPosition.y, oldPosition.x);
          targetXPosition += this->getLineOffset(oldPosition.y);
        }

        int offsetTarget = targetXPosition;
        offsetTarget -= this->getLineOffset(newY);

        int newX = this->resizableText.getColumnIndexFromPixelPosition(newY, offsetTarget);

        this->setCaretCharPosition(Point(newX, newY));
        this->arrowUpAndDownTemporaryXValue = targetXPosition;
      }
    }
    else
      assert(false && "Invalid keyEvent passed to handleArrowKey");
  }

  void TextBox::addToNextCharacter(int unichar)
  {
    char buffer[8];
    for (int i = 0; i < 8; ++i)
      buffer[i] = 0;

    size_t size = UTF8::encodeUtf8(buffer, unichar);
    if (this->getText().size() + size > this->getMaxLength())
      return;
    std::string character = buffer;

    if (character.empty())
      return;
    std::string text = this->getText();

    int index = this->getCaretCharIndex();

    UTF8::insert(text, index, character);
    this->editText(std::move(text));

    index = index + 1;
    index = this->getFont()->getRichTextHandler()->fixCursorPosition(this->resizableText.getRichTextData(), index);
    this->setCaretCharIndex(index);
  }

  std::string TextBox::getSelectedText() const
  {
    if (this->isSelectionEmpty())
      return std::string();
    return UTF8::subStr(this->getText(), this->getSelectionStart(), this->getSelectionEnd() - this->getSelectionStart());
  }

  bool TextBox::handleHotkeys(const KeyEvent& keyEvent)
  {
    switch (TextBox::getHotkeyType(keyEvent))
    {
      case HotKeyType::SelectAll:
      {
        this->selectAll();
        return true;
      }
      case HotKeyType::Copy:
      {
        if (this->getSelectionEnd() - getSelectionStart() > 0)
          SystemClipboard::copy(this->getSelectedText());
        return true;
      }
      case HotKeyType::Cut:
      {
        if (this->getSelectionEnd() - this->getSelectionStart() > 0)
        {
          SystemClipboard::copy(this->getSelectedText());

          if (!this->isReadOnly())
            this->deleteSelection(true);
        }
        return true;
      }
      case HotKeyType::Paste:
      {
        this->pasteClipboard();
        return true;
      }
      case HotKeyType::None: break;
    }

    return false;
  }

  void TextBox::deleteSelection(bool dispatchEvent)
  {
    if (this->isSelectionEmpty())
      return;

    RichTextHandler::DeleteCharsResult result = this->resizableText.deleteChars(
          this->getCaretCharIndex(),
          this->selectionIndexes,
          TextMoveDirection::Forwards,
          TextMoveType::SingleChar);
    if (dispatchEvent)
      this->editText(std::move(result.newText));
    else
      this->setText(std::move(result.newText));
    this->setCaretCharIndex(result.newCursorIndex);
  }

  void TextBox::setSelection(int startIndex, int endIndex)
  {
    // swap so that the order is smallest to largest
    if (startIndex > endIndex)
    {
      int temp = startIndex;
      startIndex = endIndex;
      endIndex = temp;
    }

    if (!this->isSelectable())
    {
      startIndex = 0;
      endIndex = 0;
    }

    this->selectionIndexes = Point(startIndex, endIndex);

    this->selectionRectanglesCache.reset();
  }

  const std::vector<std::pair<Point, Point>>& TextBox::getSelectionRectangles() const
  {
    if (!this->selectionRectanglesCache.has_value())
    {
      std::vector<std::pair<Point, Point>>& rectangles = this->selectionRectanglesCache.emplace();

      int startIndex = this->selectionIndexes.x;
      int endIndex = this->selectionIndexes.y;
      if (startIndex != 0 || endIndex != 0)
      {
        startIndex = std::max(0, startIndex);
        endIndex = std::min(endIndex, int(UTF8::length(this->getText())));

        // only highlight the selection lines that are visible
        int visibleStart = this->getVisibleLineStart();
        int visibleEnd = visibleStart + this->getVisibleLineCount();

        Point startColRow = this->charPositionFromIndex(startIndex);
        Point endColRow = this->charPositionFromIndex(endIndex);

        int rowBegin = visibleStart > startColRow.y ? visibleStart : startColRow.y;
        int rowEnd = visibleEnd < endColRow.y ? visibleEnd : endColRow.y;

        for (int row = rowBegin; row <= rowEnd; row++)
        {
          int startColumnForThisRow = 0;
          if (row == startColRow.y)
            startColumnForThisRow = startColRow.x;

          int endColumnForThisRow = int(UTF8::length(this->resizableText.lines()[row]));
          if (row == endColRow.y)
            endColumnForThisRow = endColRow.x;

          int selectLeft = this->getLineWidth(row, startColumnForThisRow);
          int selectRight = this->getLineWidth(row, endColumnForThisRow);

          Point topLeft(selectLeft + this->getLineOffset(row), this->getLineHeight() * row);
          Point widthHeight(selectRight - selectLeft, this->getLineHeight());

          rectangles.emplace_back(topLeft, widthHeight);
        }
      }
    }
    return *this->selectionRectanglesCache;
  }

  int TextBox::getLineWidth(int row, int endIndex) const
  {
    if (endIndex == std::numeric_limits<int>::max() && row >= 0 && row < int(this->getLinePixelMetadata().lineWidths.size()))
      return this->getLinePixelMetadata().lineWidths[row];
    return this->resizableText.getSubstringWidth(row, endIndex);
  }

  TextHighlightErrorColors TextBox::getHighlightColors() const
  {
    if (this->isFocused() && !this->isReadOnly())
      return TextHighlightErrorColors
      {
        this->style.getSelectedRichTextHighlightErrorColor(),
        this->style.getSelectedRichTextHighlightWarningColor(),
        this->style.getSelectedRichTextHighlightOkColor()
      };

    return TextHighlightErrorColors
    {
      this->style.getRichTextHighlightErrorColor(),
      this->style.getRichTextHighlightWarningColor(),
      this->style.getRichTextHighlightOkColor()
    };
  }

  int TextBox::findWordBoundaryInText(std::string_view text, int currentPosition, TextMoveDirection direction)
  {
    int offset;
    std::function<size_t(size_t&, std::string_view)> advance;

    if (direction == TextMoveDirection::Forwards)
    {
      offset = 1;
      advance = [](size_t& bytePositionIt, std::string_view text)
      {
        return UTF8::bringToNextUnichar(bytePositionIt, text);
      };
    }
    else
    {
      offset = -1;
      advance = [](size_t& bytePositionIt, std::string_view text)
      {
        return UTF8::bringToPrevUnichar(bytePositionIt, text);
      };
    }

    size_t length = UTF8::length(text);
    int start = std::max(std::min(currentPosition, int(length)), 0);

    size_t stringByteIndex = 0;
    for (int i = 0; i != start; i++)
      UTF8::bringToNextUnichar(stringByteIndex, text);

    int position = start;
    bool lastCharacterWasWhitespace = false;

    while (true)
    {
      size_t previousByteIndex = stringByteIndex;
      size_t letterLength = advance(stringByteIndex, text);

      if (letterLength == 0) // end of string
        break;

      size_t charStart = std::min(previousByteIndex, stringByteIndex);
      auto character = text.substr(charStart, letterLength);
      bool whitespace = (character.size() == 1) ? (isalnum(static_cast<unsigned char>(character[0])) == 0) : false;

      if (position != start && whitespace && !lastCharacterWasWhitespace)
        return position;

      position += offset;
      lastCharacterWasWhitespace = whitespace;
    }

    if (position <= 0)
      return 0;
    return int(UTF8::length(text));
  }

  TextBox::HotKeyType TextBox::getHotkeyType(const KeyEvent& keyEvent)
  {
#ifdef WIN32
    // On windows, altgr sends lctrl and ralt events.
    // When using a layout which uses altgr as a compose key, and typing a
    // character which is otherwise a hotkey (eg which is altgr-a in polish layout)
    // we want to avoid triggering a hotkey.
    if (keyEvent.control() && keyEvent.alt())
      return HotKeyType::None;
#endif

    bool isKeyDown = keyEvent.metaOrControl();
    if (isKeyDown && keyEvent.getKey() == KEY_A)
      return HotKeyType::SelectAll;
    if (isKeyDown && keyEvent.getKey() == KEY_C)
      return HotKeyType::Copy;
    if (isKeyDown && keyEvent.getKey() == KEY_X)
      return HotKeyType::Cut;
    if (isKeyDown && keyEvent.getKey() == KEY_V)
      return HotKeyType::Paste;
    return HotKeyType::None;
  }
}

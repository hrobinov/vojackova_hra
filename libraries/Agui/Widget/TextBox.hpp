#pragma once
#include "Agui/BlinkingEvent.hpp"
#include "Agui/KeyboardInputBlockType.hpp"
#include "Agui/ScrollPolicy.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/ScrollBar.hpp"
#include "Agui/Widget/TextBoxStyle.hpp"
#include "Agui/TextEnums.hpp"
#include <memory>

namespace agui { class KeyboardInputBlockerBase; }

namespace agui
{
  /** Multi line TextBox.
   *
   * Its text can be highlighted (selected), it can be word wrapped or only parse new line characters,
   * and its text can be aligned LEFT, CENTER or RIGHT. */
  class TextBox
    : public Widget
    , public BlinkingEvent
  {
    using super = Widget;
  public:
    TextBox(const TextBoxStyle* parentStyle = &TextBox::defaultStyle);
    virtual ~TextBox();

    virtual Style* getStyle() override { return &this->style; }
    virtual void updateText(); // Updates the text by splitting it into lines and updates the text.
    virtual void logic(double timeElapsed) override; // Handles the blinking.
    HorizontalAlign getTextAlignment() const; // @return The text alignment (LEFT, CENTER, RIGHT).
    virtual bool handlesMouseWheel(bool isShiftDown) const override;
    /** @return True if the text will be split into lines that fit the width of the TextBox. */
    bool isWordWrap() const;
    /** Sets whether or not the text will be split into lines that fit the width of the TextBox. */
    void setWordWrap(bool wordWrap);
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual void flagsChanged() override;
    virtual void focusGained(TabbedIn tabbedIn) override;
    virtual void focusLost() override;
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;
    virtual bool mouseWheelDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent) override;
    virtual TextInputInfo queryTextInputInfo() override;
    virtual void scrollToCaret(); // Scrolls to the caret if needed.
    virtual void setText(const std::string& text) override;
    virtual void setText(std::string&& text) override;
    virtual void setHScrollPolicy(ScrollPolicy policy);
    virtual void setVScrollPolicy(ScrollPolicy policy);
    virtual ScrollPolicy getHScrollPolicy() const;
    virtual ScrollPolicy getVScrollPolicy() const;
    virtual bool isHScrollNeeded() const;
    virtual bool isVScrollNeeded() const;
    virtual bool isTextBox() const override { return true; }
    void scrollTo(int x, int y);
    void scrollToH(int x);
    void scrollToV(int x);
    int getCurrentScrollX() const;
    int getCurrentScrollY() const;
    int getMinScrollH() const;
    int getMinScrollV() const;
    int getMaxScrollH() const;
    int getMaxScrollV() const;
    int getWidestUnwrappableLineWidth() const; // @return Width (in pixels) of the widest line of text, or 0 if wrapping is allowed
    int getWidestLineWidth() const;
    virtual int getLinesHeight() const; // @return Height (in pixels) of all lines of text, combined
    virtual void resizeToContents() override; // Resizes to fit the height if word wrapped, otherwise resizes to fit the width and height.
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    virtual KeyboardInputBlockType getKeyboardInputBlockType() const;
    virtual void setKeyboardInputBlockType(KeyboardInputBlockType type);
    virtual int getColumnIndexFromPixelPosition(int row, int x) const;
    virtual CursorProvider::CursorEnum getEnterCursor() const override;
    virtual const ElementImageSet* getBorderImageSet() const override;
    // @return caret's position within the linear text ('\n' counted as one character), in characters
    int getCaretCharIndex() const { return this->caretCharIndex; }
    void setCaretCharIndex(int index, bool fromResize = false);
    virtual void setCaretCharPosition(const Point& cpos, bool fromResize = false); // Relocates the caret based on the Horizontal and Vertical offsets.
    virtual const std::vector<std::string_view>& getTextLines() const { return this->resizableText.lines(); }
    void setTextAndCallOnTextEdited(std::string&& text) { this->editText(std::move(text)); }
    void onTextSettingsChanged();

    /** Appends the parameter UTF8 encoded string to the TextBox.
     * @param atCurrentPosition Determines if it should be appended starting
     * at the caret position or at the end.
     * @param repositionCaret Determines if the caret should be moved to the end of the appended text. */
    void appendText(const std::string& text, bool atCurrentPosition = true, bool repositionCaret = true);

    void setSelectable(bool selectable);
    bool isSelectable() const { return this->selectable; }
    void selectAll();
    void clearSelection() { this->setSelection(0, 0); }

    void setReadOnly(bool readOnly = true) { this->readOnly = readOnly; }
    bool isReadOnly() const { return this->readOnly; }

    void setMaxLength(int maxLength) { this->maxLength = maxLength; }
    size_t getMaxLength() const { return this->maxLength; }

    static int findWordBoundaryInText(std::string_view text, int currentPosition, TextMoveDirection direction);

    void textInputHandleKeyEvent(const KeyEvent& keyEvent);
    virtual Widget* getGameControllerHoveredChildInternal() override { return (this->isReadOnly() ? nullptr : this); }
    bool pasteClipboard();

  private:
    virtual void handleKeyboard(const KeyEvent& keyEvent);

    void clearKeyboardInputBlocker();
    void createKeyboardInputBlocker();
    const Color& getCurrentFontColor();

  protected:
    enum class HotKeyType : uint8_t
    {
      None,
      SelectAll,
      Copy,
      Cut,
      Paste,
    };

    static HotKeyType getHotkeyType(const KeyEvent& keyEvent);

    virtual void setTextInternal(std::string&& text);

    int indexFromCharPosition(const Point& cpos) const; // @return Index in the text (in characters) given a column and a row.
    Point charPositionFromIndex(size_t index) const; // @return The column and row given a character index in the text.

    void editText(std::string&& text);
    // @return X offset of the left edge of the numbered line of text,
    // relative to the left edge of the text area,
    // taking into account text alignment.
    int getLineOffset(int line) const;

    int getVisibleLineStart() const; // @return The index of the first line that is visible (Used for rendering).
    int getVisibleLineCount() const;

    const Font* getFont() const;
    int getLineHeight() const;

    virtual void checkScrollPolicy(); // Enables or disables the ScrollBars based on the ScrollPolicy.
    virtual void resizeSBsToPolicy(); // Will resize the ScrollBars based on the policy.
    virtual void adjustSBRanges(); // Will adjust the ScrollBar ranges based on the content width and content height.
    virtual void updateScrollBars(); // Checks the policy, resizes the scroll bars, and adjusts the ranges.
    virtual void makeLinesFromNewline(); // Splits the text into lines only when it finds a newline character.
    virtual void makeLinesFromWordWrap(); // Splits the text into lines when the width of the line exceeds the width of the TextBox.
    int getVerticalOffset() const;
    int getHorizontalOffset() const;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    void paintText(const PaintEvent& paintEvent, const agui::Point& absolutPosition, const std::string* overrideText, int textStartY);
    virtual void paintCursor(const PaintEvent& paintEvent, Point topLeft);
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point&) override;
    virtual const ElementImageSet* chooseBackground() const;
    virtual Point pixelToCharPosition(const Point& ppos) const; // Finds which character the mouse is on.
    virtual bool canPasteText(std::string_view text) const { (void)text; return true; }
    virtual void displaySizeChanged() override;

    Point findLineEdge(int currentPosition, TextMoveDirection direction);
    void handleArrowKey(const KeyEvent& keyEvent);
    void addToNextCharacter(int unichar); // Adds the UTF32 character, as UTF8, in front of the caret.
    std::string getSelectedText() const; // @return The UTF8 encoded string representing the selection.
    bool handleHotkeys(const KeyEvent& keyEvent);
    void deleteSelection(bool dispatchEvent);
    virtual int getWidthForTextModifier() const { return 0; }

  public:
    /** Sets the selected / highlighted text given a zero based start index (in UTF8 characters)
     * and a zero based end index (in UTF8 characters). (So to highlight the first character call with (0, 1) ). */
    void setSelection(int startIndex, int endIndex);
    void restoreLastSelection() { this->setSelection(this->stashedSelectionIndexes.x, this->stashedSelectionIndexes.y); }

  protected:
    virtual int getLineWidth(int row, int endIndex = std::numeric_limits<int>::max()) const;

  public:
    int getSelectionStart() const { return this->selectionIndexes.x; }
    int getSelectionEnd() const { return this->selectionIndexes.y; }
    bool isSelectionEmpty() const { return this->getSelectionStart() == this->getSelectionEnd(); }
  protected:
    TextHighlightErrorColors getHighlightColors() const;

  public:
    virtual const std::string& getText() const override;
    virtual int getTextLength() const override;

  protected:
    bool dragged = false;
    int mouseDownIndex = 0;
    bool mouseIsInside = false;

    struct LineCharMetadata {
      std::vector<size_t> lineByteIndexes; // Byte index of beginning of each line
      std::vector<size_t> lineCharIndexes; // Character index of beginning of each line
      std::vector<size_t> lineLengths; // Lengths of lines, in characters
    };
    mutable std::optional<LineCharMetadata> lineCharMetadataCache;
    const LineCharMetadata& getLineCharMetadata() const;

    struct LinePixelMetadata {
      int widestLineWidth;
      std::vector<int> lineWidths;
    };
    mutable std::optional<LinePixelMetadata> linePixelMetadataCache;
    const LinePixelMetadata& getLinePixelMetadata() const;

    int caretCharIndex = 0;
    mutable std::optional<Point> caretCharPositionCache;
    mutable std::optional<Point> caretPixelPositionCache;
    Point getCaretCharPosition() const;
    Point getCaretPixelPosition() const;

    int arrowUpAndDownTemporaryXValue = -1;
    int rememberedVerticalPosition = -1;

    bool wordWrap = false;
    HorizontalAlign lastAlign = HorizontalAlign::Left;

    ScrollPolicy hScrollPolicy = ScrollPolicy::Auto;
    ScrollPolicy vScrollPolicy = ScrollPolicy::Auto;
    HorizontalScrollBar hScrollBar;
    VerticalScrollBar vScrollBar;

  public:
    KeyboardInputBlockerBase* keyboardInputBlocker = nullptr;
  protected:
    KeyboardInputBlockType keyboardBlockType = KeyboardInputBlockType::AlphaNumerical;

    Point selectionIndexes, stashedSelectionIndexes;
    bool selectable = true;
    bool readOnly = false;
    mutable std::optional<std::vector<std::pair<Point, Point>>> selectionRectanglesCache;
    const std::vector<std::pair<Point, Point>>& getSelectionRectangles() const;

    bool filterNewlinesInPaste = false;
    size_t maxLength = static_cast<size_t>(std::numeric_limits<int>::max());
  public:
    static TextBoxStyle defaultStyle;
    TextBoxStyle style;
  protected:
    ResizableText resizableText;
    Rectangle textClippingRect;

    void validate();

    bool boxLayoutIsValid = false; // Are cached values relating to the box valid?
    bool scrollToCaretDesired = false; // Should validate() scroll to caret?
  public:
    bool focusOnNextLogic = false;
    bool confirmOnEnter = false; // shift enter should make normal enter then
  };
}

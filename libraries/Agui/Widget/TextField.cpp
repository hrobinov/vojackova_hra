#include <Agui/Widget/TextField.hpp>
#include <Agui/Font.hpp>
#include <Agui/TextInputInfo.hpp>
#include <Agui/PaintEvent.hpp>
#include <Agui/Graphics.hpp>
#include <algorithm>
#include <Agui/Gui.hpp>
#include <Agui/Input.hpp>

namespace agui
{
  TextField::TextField(const TextBoxStyle* parentStyle)
    : TextBox(parentStyle)
  {
    this->setVScrollPolicy(ScrollPolicy::Never);
    this->setHScrollPolicy(ScrollPolicy::DontShowButAllowScrolling);

    this->positionCaret(0);
    this->filterNewlinesInPaste = true;
  }

  TextField::TextField(const std::string& text, const TextBoxStyle* parentStyle)
    : TextField(parentStyle)
  {
    this->setText(text);
  }

  TextField::TextField(std::string&& text, const TextBoxStyle* parentStyle)
    : TextField(parentStyle)
  {
    this->setText(std::move(text));
  }

  TextField::~TextField() = default;

  int TextField::getColumnIndexFromPixelPosition(int row, int x) const
  {
    if (this->isPassword)
      return this->style.getFont()->getStringIndexFromPosition(this->getPasswordString(), x, this->resizableText.getRichTextSetting());

    return super::getColumnIndexFromPixelPosition(row, x);
  }

  std::string TextField::getPasswordString() const
  {
    std::string passwordString;

    size_t length = size_t(std::max(this->resizableText.getTextLength(), 0));
    passwordString.reserve(length);
    for (size_t i = 0; i < length; i++)
      passwordString.push_back('*');

    return passwordString;
  }

  int TextField::getLineWidth(int row, int endIndex) const
  {
    if (this->isPassword)
      return this->style.getFont()->getSubstringWidth(this->getPasswordString(), endIndex, this->resizableText.getRichTextSetting());

    return super::getLineWidth(row, endIndex);
  }

  bool TextField::canPasteText(std::string_view textToPaste) const
  {
    if (!this->isNumeric)
      return true;

    std::string_view text[2];
    int32_t carretIndex = this->getCaretCharIndex();

    bool minusAllowed = this->numericAllowNegative;
    bool decimalAllowed = this->numericAllowDecimal;
    if (this->isSelectionEmpty())
      text[0] = this->getText();
    else
    {
      int32_t selBegin = this->getSelectionStart();
      int32_t selEnd = this->getSelectionEnd();

      carretIndex = selBegin;

      std::string_view all = this->getText();
      text[0] = all.substr(0, selBegin);
      text[1] = all.substr(selEnd);
    }

    for (std::string_view str : text)
    {
      if (carretIndex > 0 || (!str.empty() && str.front() == '-'))
        minusAllowed = false;

      if (decimalAllowed)
        decimalAllowed = (str.find('.') == std::string_view::npos);
    }

    for (size_t i = 0; i < textToPaste.size(); ++i)
    {
      char c = textToPaste[i];
      if (c >= '0' && c <= '9')
        continue;
      if (c == '-' && minusAllowed && i == 0)
        continue;
      if (c == '.' && decimalAllowed)
      {
        decimalAllowed = false; // only one decimal
        continue;
      }

      return false;
    }

    return true;
  }

  void TextField::handleKeyboard(const KeyEvent& keyEvent)
  {
    if (keyEvent.getKey() == KEY_ENTER)
    {
      if (this->dispatchConfirm(keyEvent) && this->loseFocusOnConfirm)
        this->clearFocus();
      return;
    }

    constexpr int DecimalPoint = '.'; // 0x2e
    int unicharKey = keyEvent.getUnichar();

    auto hasCharacterOutsideOfSelection = [this](char character)
    {
      const std::string& text = this->getText();
      for (size_t i = 0; i < text.length(); i++)
        if (text[i] == character && (int(i) < this->getSelectionStart() || int(i) >= this->getSelectionEnd()))
          return true;

      return false;
    };

    bool passKeyEvent = true;
    if (unicharKey >= ' ' && this->isNumeric)
    {
      passKeyEvent = false;

      if (unicharKey >= '0' && unicharKey <= '9')
        passKeyEvent = true;
      else if (this->numericAllowDecimal && unicharKey == DecimalPoint)
      {
        // check if there is already a decimal
        if (!hasCharacterOutsideOfSelection(char(DecimalPoint)))
          passKeyEvent = true;
      }
      else if (this->numericAllowNegative && unicharKey == '-')
      {
        // check if we going to edit the first position
        if (this->getCaretCharIndex() == 0 || (this->getSelectionStart() == 0 && this->getSelectionEnd() > 0))
          // check if there is already a minus outside our selected text
          if (!hasCharacterOutsideOfSelection('-'))
            passKeyEvent = true;
      }
      else if (keyEvent.getExtendedKey() != EXT_KEY_NONE)
        passKeyEvent = true;
      else if (TextBox::getHotkeyType(keyEvent) != HotKeyType::None)
        passKeyEvent = true;
      else if (keyEvent.getKey() == KEY_BACKSPACE || keyEvent.getKey() == KEY_DELETE)
        passKeyEvent = true;
    }

    if (passKeyEvent)
      this->textInputHandleKeyEvent(keyEvent);
  }

  TextInputInfo TextField::queryTextInputInfo()
  {
    TextInputInfo info = super::queryTextInputInfo();
    info.isNumeric = this->isNumeric;
    info.numericAllowDecimal = this->numericAllowDecimal;
    info.numericAllowNegative = this->numericAllowNegative;
    info.isPassword = this->isPassword;
    info.isMultiline = false;
    return info;
  }

  void TextField::setSizeToFit(const std::string& text)
  {
    if (this->style.isHorizontallyStretchable() != StretchRule::Off)
      this->style.setMinimalWidth(std::max(this->style.getMinimalWidth(),
                                           this->style.getFont()->getTextWidth(text, this->resizableText.getRichTextSetting()) + this->getHorizontalPaddings()));
    else
      this->style.setConstantWidth(std::max(this->style.getMinimalWidth(),
                                            this->style.getFont()->getTextWidth(text, this->resizableText.getRichTextSetting()) + this->getHorizontalPaddings()));
  }

  void TextField::displaySizeChanged()
  {
    super::displaySizeChanged();
    this->setSize(0, 0);
  }

  void TextField::setIsPassword(bool value)
  {
    if (this->isPassword == value)
      return;
    this->isPassword = value;

    this->triggerResize();
  }

  void TextField::setNumeric(bool numeric, bool wantDecimal, bool wantMinus)
  {
    this->isNumeric = numeric;
    this->numericAllowDecimal = wantDecimal;
    this->numericAllowNegative = wantMinus;
    if (this->keyboardBlockType == KeyboardInputBlockType::AlphaNumerical && this->isNumeric)
      this->setKeyboardInputBlockType(agui::KeyboardInputBlockType::NumbersOnly);
    else if (this->keyboardBlockType == KeyboardInputBlockType::NumbersOnly && !this->isNumeric)
      this->setKeyboardInputBlockType(KeyboardInputBlockType::AlphaNumerical);
  }

  void TextField::resizeToContents()
  {
    if (this->isHorizontallyStretchable())
    {
      Widget::setSize(0, 0);
      return;
    }

    if (this->isReadOnly())
      this->setContentSize(this->style.getFont()->getTextWidth(this->getText(), this->resizableText.getRichTextSetting()), this->style.getFont()->getLineHeight());
  }

  void TextField::paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    int textY = (this->getContentHeight() - this->style.getFont()->getLineHeight()) / 2;

    if (this->isPassword)
    {
      std::string passwordString = this->getPasswordString();
      this->paintText(paintEvent, absolutePosition, &passwordString, textY);
    }
    else
      this->paintText(paintEvent, absolutePosition, nullptr, textY);
  }

  void TextField::paintCursor(const PaintEvent& paintEvent, Point topLeft)
  {
    paintEvent.graphics()->drawLine(Point(topLeft.x, 0),
                                    Point(topLeft.x, this->getContentHeight()),
                                    this->style.getFontColor());
  }

  void TextField::setTextInternal(std::string&& text)
  {
    std::replace(text.begin(), text.end(), '\n', ' ');
    super::setTextInternal(std::move(text));
  }
}

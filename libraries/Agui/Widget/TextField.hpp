#pragma once
#include "Agui/Widget/TextBoxStyle.hpp"
#include "Agui/Widget/TextBox.hpp"

namespace agui
{
  class ElementImageSet;
  class KeyboardInputBlockerBase;
}

namespace agui
{
  /** The text can be highlighted (selected), it can contain a password,
   * and its text can be aligned LEFT, CENTER or RIGHT.
   * It can also filter keys such as only accepting numeric input. */
  class TextField : public TextBox
  {
    using super = TextBox;
  public:
    TextField(const TextBoxStyle* parentStyle = &TextField::defaultStyle);
    TextField(const std::string& text, const TextBoxStyle* parentStyle = &TextField::defaultStyle);
    TextField(std::string&& text, const TextBoxStyle* parentStyle = &TextField::defaultStyle);
    virtual ~TextField();

    using super::setCaretCharPosition;
    void positionCaret(int position) { this->setCaretCharPosition(Point(position, 0)); }
    virtual void setNumeric(bool numeric, bool wantDecimal = false, bool wantMinus = false);
    virtual void resizeToContents() override; // Sets the width and height of the TextField to fit the text
    virtual TextInputInfo queryTextInputInfo() override;
    void setSizeToFit(const std::string& text);
    virtual void displaySizeChanged() override;
    bool getIsNumeric() const { return this->isNumeric; }
    void setIsPassword(bool value);
    bool getIsPassword() const { return this->isPassword; }

  protected:
    virtual void setTextInternal(std::string&& text) override;
    virtual void handleKeyboard(const KeyEvent& keyEvent) override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintCursor(const PaintEvent& paintEvent, Point topLeft) override;
    virtual int getColumnIndexFromPixelPosition(int row, int x) const override;
    std::string getPasswordString() const;
    virtual int getLineWidth(int row, int endIndex = std::numeric_limits<int>::max()) const override;
    virtual bool canPasteText(std::string_view text) const override;
    // These need to be changed
    bool isNumeric = false;
    bool numericAllowDecimal = false;
    bool numericAllowNegative = false;
    bool isPassword = false;
  public:
    bool loseFocusOnConfirm = false;
  };
}

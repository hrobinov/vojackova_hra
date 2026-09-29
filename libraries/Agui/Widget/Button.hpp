#pragma once
#include "Agui/Clickable.hpp"
#include "Agui/Widget/ButtonStyle.hpp"

namespace agui
{
  /** Button that can be pushed or toggled. */
  class Button : public Clickable
  {
    using super = Clickable;
  public:
    Button(const ButtonStyle* parentStyle = &Button::defaultStyle);
    virtual Style* getStyle() override { return &this->style; }
    virtual Button* asButton() override { return this; }
    virtual const Button* asButton() const override { return this; }
    virtual bool shouldClipRendering() const override;
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void modifyIsToggled(bool toggled); // Internally changes the toggled value.

  protected:
    virtual void changeClickState(Clickable::ClickState state) override;
    virtual void handleToggleClick(bool leftClick = true); // Changes the toggled value to NOT toggled value and changes the button state to reflect it.
  public:
    virtual void modifyClickState() override;
    virtual void setToggleState(bool toggled); // Manually changes the toggle state.
    virtual bool isToggleButton() const override; // @return True if this button can be toggled.
    virtual bool isToggled() const override;
    bool isToggledInternal() const { return this->toggled; }
    void setToggleButton(bool toggleButton);
    Widget& startToggle(bool toggled);
    bool isAutoUntoggling() const; // @return whether the button will untoggle itself when it is toggled and then clicked.
    void setAutoUntoggle(bool untoggle); // Sets if the button will untoggle itself when it is toggled and then clicked.
    void setButtonState(Clickable::ClickState state); // Manually sets the button state. Will be changed when the button gets an event.
    void resetState();
    virtual void focusGained(TabbedIn tabbedIn) override;
    virtual void focusLost() override;
    virtual bool shouldPlaySound() const override { return !this->toggled || this->autoUntoggle; }
    virtual const ElementImageSet* getBorderImageSet() const override;

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseClick(const MouseEvent& mouseEvent) override;
    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyUp(const KeyEvent& keyEvent) override;
    int currentVerticalOffset() const;
    virtual bool matchesSearch(const std::string&) const { return false; }

    void addRemark(const std::string& remarkText);
    void addRemark(agui::Label* remark);
    const ElementImageSet* getCurrentImageSet() const;
    Color getFontColor() const;
    void updateLabelFontColor();
    virtual Widget& setEnabled(bool enabled) override;

    static ButtonStyle defaultStyle;
  private:
    bool isDoingKeyAction = false;
    bool isButtonToggleButton = false;
    bool toggled = false;
    bool autoUntoggle = false;
    bool inputDisabled = false;
  public:
    bool strikethrough = false;
    ButtonStyle style;
  };
}

#pragma once
#include "Agui/Clickable.hpp"
#include "Agui/Widget.hpp"
#include "Agui/ResizableText.hpp"

namespace agui
{
  /** ToggleButton that can be CHECKED, UNCHECKED, or INTERMEDIATE. */
  class ToggleButton : public Clickable
  {
    using super = Clickable;
  public:

    enum class CheckedState
    {
      UNCHECKED,
      CHECKED,
      INTERMEDIATE,
    };

    ToggleButton();
    ToggleButton(std::string&& text);
    virtual ~ToggleButton();

  protected:
    virtual void drawItem(const PaintEvent& paintEvent, const  Rectangle& rectangle) const = 0;
    virtual void drawText(const PaintEvent& paintEvent) const;
    virtual const ElementImageSet* getBackgroundGraphicalSet() const = 0;

    virtual const Rectangle& getWordWrapRect() const; // @return Rectangle provided when drawing the text.
    void changeCheckedState(ToggleButton::CheckedState state);
    virtual void nextCheckState(); // Internally changes to the next logical checked state.
    virtual void positionButton(); // Generates the Rectangle used to draw the ToggleButton.
    virtual const Point& getButtonPosition() const; // @return The position of the actual ToggleButton.
    virtual void resizeCaption(); // Internally resizes the caption text.
    virtual const Rectangle& getButtonRectangle() const; // @return The Rectangle used to draw the ToggleButton itself.
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual Dimension getDimension() const { return { 0, 0 }; }

    // for passing style from children
    virtual const Font* getFont() const = 0;
    virtual const Color& getFontColor() const = 0;
    virtual HorizontalAlign getHorizontalAlign() const = 0;
    virtual VerticalAlign getVerticalAlign() const = 0;
    virtual uint32_t getTextPadding() const = 0;
  public:
    void onCheckChange(GenericTargetable* owner, std::function<void(bool)> callback);
    void onCheckChange(GenericTargetable* owner, std::function<void()> callback);
    void onCheck(GenericTargetable* owner, std::function<void()> callback);
    virtual int getMiddleGap() const;
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual void setText(const std::string& text) override;
    virtual void setText(std::string&& text) override;
    virtual bool mouseDown(const MouseEvent&) override;
    // This sending the action listener event to this, so the ToggleButton works properly when used with Widget::isFor
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    //virtual void onMouseClick(const MouseEvent& mouseEvent) override { this->mouseClick(mouseEvent); }
    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyUp(const KeyEvent& keyEvent) override;

    void setChecked(bool checked = true); // Sets whether or not the ToggleButton is checked.
    void setCheckedState(CheckedState state);
    bool isChecked() const; // @return True if the ToggleButton is checked.
    virtual Dimension getButtonSize() const; // @return The size of the actual ToggleButton.
    virtual void resizeToContents() override; // Resizes the ToggleButton to a size that fits the caption nicely.
    ToggleButton::CheckedState getCheckedState() const;
    bool dispatchCheck();
    bool dispatchCheckChange();

  protected:
    Rectangle wordWrapRect;
    std::string trimmedLine;
    Point buttonPosition;
    Rectangle buttonRect;
    AreaAlign textAlignment = AreaAlign::LeftMiddle;
    AreaAlign buttonAlignment = AreaAlign::LeftMiddle;
    CheckedState checkedState = CheckedState::UNCHECKED;
    bool isDoingKeyAction = false;
  };
}

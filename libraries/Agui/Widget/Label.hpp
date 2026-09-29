#pragma once
#include "Agui/Widget.hpp"
#include "Agui/ResizableText.hpp"
#include "Agui/Widget/LabelStyle.hpp"
#include "Agui/ResizableText.hpp"
namespace agui { class Window; }

namespace agui
{
  class Label : public Widget
  {
    using super = Widget;
  public:
    using StyleType = LabelStyle;

    Label(const StyleType* parentStyle = &Label::defaultStyle);
    Label(RichTextSetting richTextSetting, const StyleType* parentStyle = &Label::defaultStyle);
    Label(const std::string& text, const StyleType* parentStyle = &Label::defaultStyle);
    Label(const std::string& text, RichTextSetting richTextSetting, const StyleType* parentStyle = &Label::defaultStyle);
    Label(std::string&& text, const StyleType* parentStyle = &Label::defaultStyle);
    Label(std::string&& text, const StyleType* parentStyle, RichTextSetting richTextSetting);
    Label(std::string&& text, RichTextSetting richTextSetting, const StyleType* parentStyle = &Label::defaultStyle);
    virtual ~Label();

  private:
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    Color getTextColor();
    virtual void drawText(const PaintEvent& paintEvent);

  public:
    void updateLabel(bool reactionToSetSize); // Updates the text of the label.

    virtual Style* getStyle() override { return &this->style; }
    Label& multiLine() { this->setSingleLine(false); return *this; }
    virtual void setSingleLine(bool singleLine);
    virtual bool isSingleLine() const;
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    virtual void setText(const std::string& text) override;
    virtual void setText(std::string&& text) override;
    void setText(const char*) = delete;
    virtual void resizeToContents() override; // Resizes the Label to fit the caption text.
    int getNumTextLines() const;
    int getRequiredWidth();
    int getRequiredHeight();
    int getWidestTextLineWidth() const;
    void setSizeToFit(std::string_view text);
    static int32_t getWidthToFit(const LabelStyle* labelStyle, std::string_view text);
    void onRichTextSettingsChanged();
    virtual void displaySizeChanged() override;
    virtual bool genericSearch(const LowercaseString& filter) override;

    virtual Window* getDragTarget() override;
    virtual const Window* getDragTarget() const override;
    virtual void setDragTarget(Window* dragTarget);

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual bool dragOnlyByLeftMouseButton() const override { return true; }
    virtual const std::string& getText() const override { return this->resizableText.str(); }
    virtual ToolTip* createToolTip() override;
    TextHighlightErrorColors getHighlightColors() const;
    const ResizableText& getResizableText() const { return this->resizableText; }
    ResizableText& getResizableText() { return this->resizableText; }
    virtual Widget* getGameControllerHoveredChildInternal() override { return this->hasToolTipCreator() ? this : nullptr; }
    bool getMouseIsOver() { return this->mouseIsOver; }

    static StyleType defaultStyle;
    StyleType style;
  private:
    Window* dragTarget = nullptr;
    bool mouseIsOver = false;
    ResizableText resizableText;
    std::string singleLineData;
  };
}

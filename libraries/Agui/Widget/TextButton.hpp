#pragma once
#include "Agui/ResizableText.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/Button.hpp"
namespace agui { class ElementImageSet; }

namespace agui
{
  /** Button that can be pushed or toggled. */
  class TextButton : public Button
  {
    using super = Button;
  protected:
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    int getTextHeight();
    int calculateContentHeight(); // useful before height is set
  public:
    TextButton(const ButtonStyle* parentStyle = &Button::defaultStyle);
    TextButton(std::string&& text, const ButtonStyle* parentStyle = &Button::defaultStyle);
    virtual void resizeToContents() override; // Resizes the button to fit the width and height of the text + margins.
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual ~TextButton() = default;
    void setTextAlignment(AreaAlign alignment);
    AreaAlign getTextAlignment() const;
    virtual int getTextOffset() { return 0; } // creates space left of the text; used for an icon
    //true if the text is too long to fit the button. It's used internally but can be used by custom tooltip logic also.
    bool shouldShowTooltip();
    Widget* getRemark();
    virtual ToolTip* createToolTip() override;
    virtual bool genericSearch(const LowercaseString& filter) override;

  private:
    AreaAlign textAlignment = AreaAlign::CenterMiddle;
  };
}

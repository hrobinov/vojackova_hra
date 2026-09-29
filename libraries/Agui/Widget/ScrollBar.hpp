#pragma once
#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/TextButton.hpp"
#include "Agui/Widget/ScrollBarStyle.hpp"

namespace agui
{
  class HorizontalPolicy
  {
  public:
    static ScrollBarStyle defaultStyle;
    static int getCoordinate(Point point) { return point.x; }
    static int getDimension(const Widget& widget) { return widget.getWidth(); }
    static int getContentDimension(const Widget& widget) { return widget.getContentWidth(); }
    static void setSize(Button& thumb, int32_t size, int32_t minimalSize);
    static void setLocation(Widget& thumb, int location, const Dimension holderSize);
  };

  class VerticalPolicy
  {
  public:
    static ScrollBarStyle defaultStyle;
    static int getCoordinate(Point p) { return p.y; }
    static int getDimension(const Widget& w) { return w.getHeight(); }
    static int getContentDimension(const Widget& w) { return w.getContentHeight(); }
    static void setSize(Button& thumb, int32_t size, int32_t minimalSize);
    static void setLocation(Widget& thumb, int location, const Dimension holderSize);
  };

  template<typename Policy>
  class ScrollBar : public Widget
  {
    using super = Widget;
  protected:
    virtual void resizeThumb(); // Resizes the thumb to fit page requirements.
    virtual void positionThumb(); // Positions the thumb on resize.
    int getAdjustedMaxThumbSize() const; // @return The maximum thumb size with constraint considerations.
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;

  public:
    using PolicyType = Policy;
    ScrollBar(const ScrollBarStyle* parentStyle = &Policy::defaultStyle);
    virtual Style* getStyle() override { return &this->style; }
    virtual bool isScrollBar() const override { return true; }
    virtual void reapplySubStyles() override;
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseDoubleClick(const MouseEvent& mouseEvent) override;
    void setMouseWheelAmount(int amount); // Sets the amount to scroll in wheelScrollUp and wheelScrollDown.
    int getMouseWheelAmount() const; // @return The amount to scroll in wheelScrollUp and wheelScrollDown.
    /** Moves the ScrollBar by the amount set in setMouseWheelAmount and the deltaWheel parameter.
     * DeltaWheel is NOT expected to be an absolute value. */
    virtual void wheelScrollDown(int deltaWheel);
    /** Moves the ScrollBar by the amount set in setMouseWheelAmount and
     * the deltaWheel parameter. deltaWheel is NOT expected to be an absolute value. */
    virtual void wheelScrollUp(int deltaWheel);
    /** Moves the ScrollBar by the amount set in setMouseWheelAmount and
     * the deltaWheel parameter. deltaWheel is NOT expected to be an absolute value. */
    virtual void wheelScrollLeft(int deltaWheel);
    /** Moves the ScrollBar by the amount set in setMouseWheelAmount and
     * the deltaWheel parameter. deltaWheel is NOT expected to be an absolute value. */
    virtual void wheelScrollRight(int deltaWheel);
    // @return True if the thumb is at the maximum value, it should stick there when the bar is resized. Useful for chat TextBoxes
    bool isStickingToBottom() const;
    // Sets whether or not if the thumb is at the maximum value, it should stick there when the bar is resized. Useful for chat TextBoxes.
    void setStickToBottom();
    // Adjusts the scrollbar so that the largeAmount is pageSize and there are enough pages to fill the content size.
    void setRangeFromPage(int pageSize, int contentSize);
    void setLargeAmount(int amount); // Sets the large amount. This is often the size of the thumb, or the size of a page.
    void setValue(int val, bool dispatchChange = false); // Sets the value of the scroll bar. Must be between min and max value.
    void scrollToCenter();
    void setMinValue(int val); // Sets the minimum value of the scroll bar. Can be negative.
    void setMaxValue(int val); // Sets the maximum value of the scroll bar. Can be negative.
    virtual bool mouseWheelDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelLeft(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelRight(const MouseEvent& mouseEvent) override;
    int getValueFromPosition(int position) const; // @return The value of the scrollbar when the
    /** @return Value from 0.0f to 1.0f indicating how far into the values the thumb is.
     * When the thumb is at min value it returns 0.0f, if it is at max value, 1.0f. */
    float getRelativeValue() const;
    int getLargeAmount() const; // @return The large amount. This is often the size of the thumb, or the size of a page.
    int getValue() const;
    int getMinValue() const;
    int getMaxValue() const;
    int getMaxUsefulValue() const;

    int getMaxThumbSize() const; // @return The maximum size of the thumb.
    virtual bool isThumbAtStart() const; // @return True if the thumb is completely at the start.
    virtual bool isThumbAtEnd() const; // @return True if the thumb is completely at the end.
    int getMinThumbSize() const;
    virtual void setMinThumbSize(int size);
    virtual void resizeToContents() override {}
    void onSliderMove(GenericTargetable* owner, std::function<void(double)> callback);
    void onSliderMove(GenericTargetable* owner, std::function<void()> callback);

    ScrollBarStyle style;
    bool reactToThumbDrag = true;

  private:
    int largeAmount = 10;
    int minValue = 0;
    int maxValue = 100;
    int wheelSpeed = 1;
    int currentValue = 0;

    int downThumbPos = 0;
    int downMousePos = 0;

    int minThumbSize = 30;
    bool stickToBottom = false;

    Button thumb;
  };

  using HorizontalScrollBar = ScrollBar<HorizontalPolicy>;
  using VerticalScrollBar = ScrollBar<VerticalPolicy>;
}

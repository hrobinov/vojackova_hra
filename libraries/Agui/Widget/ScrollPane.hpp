#pragma once
#include "Agui/ScrollPolicy.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/ScrollBar.hpp"
#include "Agui/Widget/ScrollPaneStyle.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include "Agui/ScrollMode.hpp"
namespace agui
{
  class HorizontalPolicy;
  class VerticalPolicy;
  template<typename Policy> class ScrollBar;
  using HorizontalScrollBar = ScrollBar<HorizontalPolicy>;
  using VerticalScrollBar = ScrollBar<VerticalPolicy>;
}

namespace agui
{
/** ScrollPane to scroll an area that is larger than the size of the widget.
 * The scrollPane works exactly the same as FlowLayout, unless the style.maximalHeight/maximalWidth is set and
 * the contents doesn't fit it */
  class ScrollPane : public Widget
  {
    using super = Widget;
  public:
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;

    ScrollPane(const ScrollPaneStyle* parentStyle = &ScrollPane::defaultStyle);
    ScrollPane(const ScrollPane&) = delete;
    ~ScrollPane();
    void operator=(const ScrollPane&) = delete;

    virtual void updateScrollBars(); // Checks the policy, resizes the scroll bars, and adjusts the ranges.
    virtual ScrollPane* asScrollPane() override { return this; }
    virtual const ScrollPane* asScrollPane() const override { return this; }
  protected:
    virtual void checkScrollPolicy(); // Enables or disables the ScrollBars based on the ScrollPolicy.
    virtual void resizeScrollBarssToPolicy(); // Will resize the ScrollBars based on the policy.
    virtual void adjustScrollBarRanges(); // Will adjust the ScrollBar ranges based on the content width and content height.
    virtual void checkScrollPositions();
    /** Uses arrow keys to scroll when another widget has focus.
     * You can call this in the ScrollPane's keyDown and keyRepeat events if you need it when it is focused. */
    virtual void keyAction(ExtendedKeyEnum key, bool shift);
    virtual bool mouseWheelDown(const MouseEvent& mouseEvent) override; // widget call
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent) override; // widget call
    virtual bool mouseWheelLeft(const MouseEvent& mouseEvent) override; // widget call
    virtual bool mouseWheelRight(const MouseEvent& mouseEvent) override; // widget call
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;
    void updatecontentFlowPosition();
    virtual void recalculateClippingRect() const override;

  public:
    virtual Style* getStyle() override { return &this->style; }

    virtual void reapplySubStyles() override;
    virtual void postDisplaySizeChanged() override;
    bool isPointVisible(const agui::Point&) const;
    bool isAreaVisible(const agui::Rectangle&) const; // Returns true if at least part of the are is visible.
    virtual void flagAllChildrenForDestruction() override; // Also flags the content Widget's children.
    virtual void addFront(Widget* widget) override;
    virtual void add(Widget* widget) override;
    virtual void insert(Widget* widget, uint32_t index) override;
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False) override;
    virtual void swapChildren(size_t index1, size_t index2) override;
    int getContentsWidth() const;
    int getContentsHeight() const;
    bool isHScrollNeeded() const; // @return True if the Horizontal Scrollbar is needed (Does not consider policy).
    bool isVScrollNeeded() const; // @return if the Vertical Scrollbar is needed (Does not consider policy).
    virtual bool handlesMouseWheel(bool isShiftDown) const override;
    void setHScrollPolicy(ScrollPolicy policy);
    void setVScrollPolicy(ScrollPolicy policy);
    ScrollPolicy getHScrollPolicy() const;
    ScrollPolicy getVScrollPolicy() const;
    int32_t getVerticalScrollbarWidth() const;
    /** This will resize the content widget to the content width and height and update the scrollbars.
     * Override this if that is not what you want. */
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    void scrollToMakeWidgetVisible(Widget* widget, ScrollMode scrollMode);
    void scrollToMakeAreaVisible(Rectangle area, ScrollMode scrollMode);

    void scrollToInternal(int x, int y);
    void scrollTo(int x, int y);
    void scrollToH(int x);
    void scrollToV(int y);
    void scrollToBottom() { this->scrollToV(this->getMaxScrollV()); }
    void scrollToTop() { this->scrollToV(0); }
    int getCurrentScrollX() const;
    int getCurrentScrollY() const;
    int getMinScrollH() const;
    int getMinScrollV() const;
    int getMaxScrollH() const;
    int getMaxScrollV() const;
    int getMaxUsefulScrollV() const;

    /** Sets how many values in addition to the actual delta mouse wheel,
     * the vertical scrollbar will be moved when a mouse wheel event is triggered.
     *
     * The widget under the mouse has priority.
     * If a widget like a ListBox consumes the event, it will scroll instead. */
    virtual void setWheelScrollRate(int rate);
    virtual void keepVerticallyVisibleInScrollPane(int verticalPosition, int margin) override;
    int getVerticalScrollPosition();

    /** @return How many values in addition to the actual delta mouse wheel,
     * the vertical scrollbar will be
     * moved when a mouse wheel event is triggered.
     *
     * The widget under the mouse has priority.
     * If a widget like a ListBox consumes the event, it will scroll instead. */
    virtual int getWheelScrollRate() const;

    virtual void setHKeyScrollRate(int rate); // Sets how many values the left and right keys will move the Horizontal Scrollbar.
    virtual int getHKeyScrollRate() const; // @return How many values the left and right keys will move the Horizontal Scrollbar.
    virtual void setVKeyScrollRate(int rate); // Sets how many values the up and down keys will move the Vertical Scrollbar.
    virtual int getVKeyScrollRate() const; // @return How many values the up and down keys will move the Vertical Scrollbar.
    void setHMinThumbSize(int size); // Sets the smallest the Horizontal thumb will ever be.
    int getHMinThumbSize() const; // @return The smallest the Horizontal thumb will ever be.
    void setVMinThumbSize(int size); // Sets the smallest the Vertical thumb will ever be.
    int getVMinThumbSize() const; // @return The smallest the Vertical thumb will ever be.
    virtual void resizeWidthToContents(); // Resizes the width so that the content width is fully seen without needing to scroll.
    virtual void resizeHeightToContents(); // Resizes the height so that the content height is fully seen without needing to scroll.
    /** Will resize both the width and height so that the content width and height
     * is fully seen without needing to scroll. */
    virtual void resizeToContents() override;
    virtual TransparentValue isTransparent() const override { return TransparentValue::DependsOnChildren; }
    virtual void clear() override;
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis) override;
    virtual void recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintComponent(const PaintEvent&, const agui::Point& absolutePosition) override;
    Dimension getSizeForContent(bool includePaddings, bool includeReservedSpaceForScrollBars = true) const;
    void center();
    bool isVerticalBarActivated() const;
    bool isHorizontalBarActivated() const;
    bool isActivated() const;
    virtual const ElementImageSet* getBorderImageSet() const override;
    virtual int getTopPadding() const override;
    virtual int getBottomPadding() const override;
    virtual int getLeftPadding() const override;
    virtual int getRightPadding() const override;
    virtual int getTopMargin() const override;
    virtual int getBottomMargin() const override;
    virtual int getLeftMargin() const override;
    virtual int getRightMargin() const override;
    virtual void afterResizeIsSettledAction() override;
  private:
    void centerInternal();
    void scrollToMakeWidgetVisibleInternal(Widget* widget, ScrollMode scrollMode);
    void scrollToMakePositionVisibleInternal(Rectangle rectangle, ScrollMode scrollMode);
  public:

    static ScrollPaneStyle defaultStyle;
    ScrollPaneStyle style;
    VerticalFlow contentFlow;
    GenericTargeter<Widget> scrollToWidgetOnNextResize;
    std::optional<Rectangle> scrollToAreaOnNextResize;
    ScrollMode scrollModeOfScrollToOnNextResize = ScrollMode::InView;
  private:
    ScrollPolicy hScrollPolicy = ScrollPolicy::Auto;
    ScrollPolicy vScrollPolicy = ScrollPolicy::Auto;

    int hKeyScrollRate = 6;
    int vKeyScrollRate = 6;
  public:
    HorizontalScrollBar horizontalScrollBar;
    VerticalScrollBar verticalScrollBar;
  private:
    bool shouldReserveSpaceForVerticalScrollbar() const;
    bool shouldReserveSpaceForHorizontalScrollbar() const;

    int rememberedVerticalPosition = 0;
    int rememberedHorizontalPosition = 0;
    bool centerOrdered = false;
  };
}

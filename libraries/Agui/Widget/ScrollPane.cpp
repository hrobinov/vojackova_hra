#include "Agui/DrawCulledChildren.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/ScrollBar.hpp"
#include "Agui/Widget/ScrollPane.hpp"

namespace agui
{
  ScrollPaneStyle ScrollPane::defaultStyle;

  int ScrollPane::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;
    if (this->vScrollPolicy == ScrollPolicy::Never)
     return std::min(this->contentFlow.maximumVerticalSquashSize(), this->getHeight() - this->style.getMinimalHeight());
    return this->getHeight() - this->style.getMinimalHeight();
  }

  int ScrollPane::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;
    if (this->hScrollPolicy == ScrollPolicy::Never)
    {
      int unusedSize = this->getSizeForContent(true).width - this->contentFlow.getWidth();
      return std::min(std::max(0, this->contentFlow.maximumHorizontalSquashSize() + unusedSize), this->getWidth() - this->style.getMinimalWidth());
    }
    return this->getWidth() - this->style.getMinimalWidth();
  }

  ScrollPane::ScrollPane(const ScrollPaneStyle* parentStyle)
    : style(this, parentStyle)
    , contentFlow(this->style.getVerticalFlowStyle())
    , horizontalScrollBar(this->style.getHorizontalScrollBarStyle())
    , verticalScrollBar(this->style.getVerticalScrollBarStyle())
  {
    this->addPrivateChild(&this->contentFlow);
    this->addPrivateChild(&this->horizontalScrollBar);
    this->addPrivateChild(&this->verticalScrollBar);
    this->horizontalScrollBar.onSliderMove(this,[this](double sliderValue)
    {
      this->contentFlow.setLocation(-sliderValue, this->contentFlow.getLocation().y);
      this->rememberedHorizontalPosition = sliderValue;
      this->centerOrdered = false;
    });
    this->verticalScrollBar.onSliderMove(this, [this](double sliderValue)
    {
      this->contentFlow.setLocation(this->contentFlow.getLocation().x, -sliderValue);
      this->rememberedVerticalPosition = sliderValue;
      this->centerOrdered = false;
    });

    this->updateScrollBars();
    this->setWheelScrollRate(Gui::wheelScrollRate);
  }

  ScrollPane::~ScrollPane()
  {}

  void ScrollPane::addFront(Widget* widget)
  {
    this->contentFlow.addFront(widget);
  }

  void ScrollPane::add(Widget* widget)
  {
    this->contentFlow.add(widget);
  }

  void ScrollPane::insert(Widget* widget, uint32_t index)
  {
    this->contentFlow.insert(widget, index);
  }

  void ScrollPane::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    this->contentFlow.remove(widget, keepWidgetAlive);
  }

  void ScrollPane::swapChildren(size_t index1, size_t index2)
  {
    this->contentFlow.swapChildren(index1, index2);
  }

  void ScrollPane::setHScrollPolicy(ScrollPolicy policy)
  {
    if (this->hScrollPolicy == policy)
      return;

    this->hScrollPolicy = policy;
    this->updateScrollBars();
    this->triggerResize();
  }

  void ScrollPane::setVScrollPolicy(ScrollPolicy policy)
  {
    if (this->vScrollPolicy == policy)
      return;

    this->vScrollPolicy = policy;
    this->updateScrollBars();
    this->triggerResize();
  }

  ScrollPolicy ScrollPane::getHScrollPolicy() const
  {
    return this->hScrollPolicy;
  }

  ScrollPolicy ScrollPane::getVScrollPolicy() const
  {
    return this->vScrollPolicy;
  }

  int32_t ScrollPane::getVerticalScrollbarWidth() const
  {
    return this->verticalScrollBar.getWidth();
  }

  bool ScrollPane::handlesMouseWheel(bool isShiftDown) const
  {
    return this->horizontalScrollBar.isVisible() || !isShiftDown && this->verticalScrollBar.isVisible();
  }

  void ScrollPane::checkScrollPolicy()
  {
    bool horizontalScrollNeeded = this->isHScrollNeeded();
    if (!horizontalScrollNeeded)
      this->horizontalScrollBar.setValue(0); // reset the position, as the value will still be used to position the contents
    this->horizontalScrollBar.setVisibleSilent(horizontalScrollNeeded);

    bool verticalScrollNeeded = this->isVScrollNeeded();
    if (!verticalScrollNeeded)
      this->verticalScrollBar.setValue(0); // reset the position, as the value will still be used to position the contents
    this->verticalScrollBar.setVisibleSilent(verticalScrollNeeded);
  }

  void ScrollPane::resizeScrollBarssToPolicy()
  {
    this->horizontalScrollBar.setLocation(-this->getLeftPadding(), this->getContentHeight() - this->horizontalScrollBar.getHeight() + this->getBottomPadding());

    {
      int verticalScrollX = this->getContentWidth() + this->getRightPadding();
      if (!this->style.getScrollbarsGoOutside())
        verticalScrollX -= this->verticalScrollBar.getWidth();
      this->verticalScrollBar.setLocation(verticalScrollX, -this->getTopPadding());
    }

    bool horizontalSpaceNeeded = this->shouldReserveSpaceForHorizontalScrollbar();
    bool verticalSpaceNeeded = this->verticalScrollBar.isVisible() || this->vScrollPolicy == ScrollPolicy::AutoAndReserveSpace;

    if (horizontalSpaceNeeded && verticalSpaceNeeded)
    {
      this->horizontalScrollBar.style.setConstantWidth(this->getWidth() - this->verticalScrollBar.getWidth());
      this->verticalScrollBar.style.setConstantHeight(this->getHeight() - this->horizontalScrollBar.getHeight());
    }
    else if (horizontalSpaceNeeded)
      this->horizontalScrollBar.style.setConstantWidth(this->getWidth());
    else if (verticalSpaceNeeded)
      this->verticalScrollBar.style.setConstantHeight(this->getHeight());
  }

  int ScrollPane::getContentsWidth() const
  {
    int width = 0;
    for (const Widget* widget : this->contentFlow)
      if (widget->isVisible())
        width = std::max(width, widget->getRelativeRectangle().getRight() + widget->getRightMargin());
    return width + this->contentFlow.getHorizontalPaddings();
  }

  int ScrollPane::getContentsHeight() const
  {
    int height = 0;
    for (const Widget* widget : this->contentFlow)
      if (widget->isVisible())
        height = std::max(height, widget->getRelativeRectangle().getBottom() + widget->getBottomMargin());
    return height + this->contentFlow.getVerticalPaddings();
  }

  bool ScrollPane::isHScrollNeeded() const
  {
    if (this->getHScrollPolicy() == ScrollPolicy::Never)
      return false;
    if (this->getHScrollPolicy() == ScrollPolicy::Always)
      return true;
    if (this->getContentsWidth() > this->getContentWidth())
      return true;
    if (this->getVScrollPolicy() != ScrollPolicy::Never &&
        (this->getContentsHeight() > this->getContentHeight() &&
         this->getContentsWidth() > (this->getContentWidth() - (this->style.getScrollbarsGoOutside() ? 0 : this->verticalScrollBar.getWidth()))))
      return true;
    return false;
  }

  bool ScrollPane::isVScrollNeeded() const
  {
    if (this->getVScrollPolicy() == ScrollPolicy::Never)
      return false;
    if (this->getVScrollPolicy() == ScrollPolicy::Always)
      return true;

    if (this->getContentsHeight() > this->getContentHeight())
      return true;
    if (this->getHScrollPolicy() != ScrollPolicy::Never &&
        (this->getContentsWidth() > this->getContentWidth() &&
         this->getContentsHeight() > (this->getContentHeight() - this->horizontalScrollBar.getHeight())))
      return true;
    return false;
  }

  void ScrollPane::updateScrollBars()
  {
    this->checkScrollPolicy();
    this->resizeScrollBarssToPolicy();
    this->adjustScrollBarRanges();
    this->checkScrollPositions();
  }

  void ScrollPane::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    Widget::setSize(width, height, setSizeInfo);
    this->updateScrollBars();
    // when the flow is bigger than the scroll pane, it is solved by scroll bars
    // but it can also happen that the flow is smaller then the size the scrollPane was resized to.
    // in this case, we will stretch the content flow.
    Dimension sizeForFlow(this->getSizeForContent(true));
    if (sizeForFlow.width > this->contentFlow.getWidth() || sizeForFlow.height > this->contentFlow.getHeight())
    {
      int heightBefore = this->contentFlow.getHeight();
      SetSizeInfo setSizeInfoParameter;
      setSizeInfoParameter.reactionToSetSize = ReactionToSetSize::True;
      if (sizeForFlow.width > this->contentFlow.getWidth())
        setSizeInfoParameter.horizontal = Change::Stretching;
      if (sizeForFlow.height > this->contentFlow.getHeight())
        setSizeInfoParameter.vertical = Change::Stretching;
      this->contentFlow.setSize(std::max(sizeForFlow.width, this->contentFlow.getWidth()),
                                std::max(sizeForFlow.height, this->contentFlow.getHeight()),
                                setSizeInfoParameter);

      // widget (label for example), can decrease a height when stretched, this can cause the scroll bar to not be needed anymore,
      // which can free more space for the content.
      if (heightBefore != this->contentFlow.getHeight())
      {
        this->updateScrollBars();
        Dimension sizeForFlow2 = this->getSizeForContent(true);
        if (sizeForFlow2 != sizeForFlow)
        {
          sizeForFlow = sizeForFlow2;
          this->contentFlow.setSize(std::max(sizeForFlow.width, this->contentFlow.getWidth()),
                                    std::max(sizeForFlow.height, this->contentFlow.getHeight()));
        }
      }
    }

    // content flow was stretched (as this was stretched)
    // but now this is squashed, so content flow should also get smaller to either its size, or new size of this scrollbar
    // (whatever is bigger)
    // this is to avoid weird empty scrollable space of the previous stretched height of the widget
    if (sizeForFlow.height < this->contentFlow.getHeight() &&
        this->contentFlow.isVerticallyStretchable() &&
        this->contentFlow.getSizeBeforeStretching().height < this->contentFlow.getHeight())
      this->contentFlow.setSize(this->contentFlow.getWidth(), std::max(this->contentFlow.getSizeBeforeStretching().height, sizeForFlow.height));

    // The internals were stretched beyond their "natural size" and now I don't have enough of horizontal space
    // (most often because of the scroll bar, but can be other reason.
    // before extending my own size, I un-stretch the internals to fit the the scroll pane.
    if (sizeForFlow.width < this->contentFlow.getWidth())
      if (this->contentFlow.getSizeBeforeStretching().width <= sizeForFlow.width)
        this->contentFlow.setSize(sizeForFlow.width, this->contentFlow.getHeight());
      else if (this->contentFlow.isHorizontallySquashable())
        if (int squashableSize = this->contentFlow.maximumHorizontalSquashSize())
        {
          const int heightBefore = this->contentFlow.getHeight();
          const int newWidth = std::max(sizeForFlow.width, this->contentFlow.getWidth() - squashableSize);
          this->contentFlow.setSize(newWidth,
                                    heightBefore,
                                    SetSizeInfo(Change::Squashing, Change::Nothing));

          // widget (label for example), can increase in height when squashed
          if (heightBefore < this->contentFlow.getHeight())
          {
            if (setSizeInfo.vertical == Change::Nothing)
              Widget::setSize(newWidth, this->contentFlow.getHeight(), SetSizeInfo(Change::Nothing, Change::Stretching));

            this->updateScrollBars();
            Dimension sizeForFlow2 = this->getSizeForContent(true);
            if (sizeForFlow2 != sizeForFlow)
            {
              sizeForFlow = sizeForFlow2;
              this->contentFlow.setSize(std::max(sizeForFlow.width, this->contentFlow.getWidth()),
                                        std::max(sizeForFlow.height, this->contentFlow.getHeight()));
            }
          }
        }

    if (sizeForFlow.width < this->contentFlow.getWidth() && this->shouldReserveSpaceForVerticalScrollbar())
    {
      int sizeWithProgressBar = this->contentFlow.getWidth() +
                                this->getHorizontalPaddings() +
                                this->verticalScrollBar.getWidth();
      bool contentCouldFitBeforeStretching = sizeWithProgressBar <= sizeForFlow.width;

      // This condition will get simplified once I narrow down all the possible situations through tests.
      if ((setSizeInfo.maximalWidth == 0 || setSizeInfo.maximalWidth >= sizeWithProgressBar) &&
          (this->getHScrollPolicy() == ScrollPolicy::Never &&
           (!this->isHorizontallyStretchable() || !contentCouldFitBeforeStretching) &&
           (!this->isHorizontallySquashable() || setSizeInfo.horizontal != Change::Squashing) ||
           this->getHScrollPolicy() != ScrollPolicy::Never &&
           setSizeInfo.horizontal != Change::Squashing &&
           this->style.isHorizontallyStretchable() != StretchRule::On))
        Widget::setSize(sizeWithProgressBar, height);
      else if (this->getHScrollPolicy() == ScrollPolicy::Never)
        this->contentFlow.setSize(sizeForFlow.width, this->contentFlow.getHeight());
    }
    this->updateScrollBars();
  }

  void ScrollPane::scrollToMakeWidgetVisible(Widget* widget, ScrollMode scrollMode)
  {
    this->scrollToWidgetOnNextResize = widget;
    this->scrollModeOfScrollToOnNextResize = scrollMode;
    Gui::instance->callAfterResizeIsSattledAction(this);
  }

  void ScrollPane::scrollToMakeAreaVisible(Rectangle area, ScrollMode scrollMode)
  {
    this->scrollToAreaOnNextResize = area;
    this->scrollModeOfScrollToOnNextResize = scrollMode;
    Gui::instance->callAfterResizeIsSattledAction(this);
  }

  void ScrollPane::scrollToInternal(int x, int y)
  {
    if (this->horizontalScrollBar.isVisible())
      this->horizontalScrollBar.setValue(x);
    if (this->verticalScrollBar.isVisible())
      this->verticalScrollBar.setValue(y);
    this->updatecontentFlowPosition();
  }

  void ScrollPane::scrollTo(int x, int y)
  {
    this->scrollToInternal(x, y);
    this->rememberedHorizontalPosition = x;
    this->rememberedVerticalPosition = y;
    this->centerOrdered = false;
  }

  void ScrollPane::scrollToH(int x)
  {
    this->horizontalScrollBar.setValue(x);
    this->rememberedHorizontalPosition = x;
    this->updatecontentFlowPosition();
  }

  void ScrollPane::scrollToV(int y)
  {
    this->verticalScrollBar.setValue(y);
    this->rememberedVerticalPosition = y;
    this->updatecontentFlowPosition();
  }

  int ScrollPane::getCurrentScrollX() const
  {
    return this->horizontalScrollBar.getValue();
  }

  int ScrollPane::getCurrentScrollY() const
  {
    return this->verticalScrollBar.getValue();
  }

  int ScrollPane::getMinScrollH() const
  {
    return this->horizontalScrollBar.getMinValue();
  }

  int ScrollPane::getMinScrollV() const
  {
    return this->verticalScrollBar.getMinValue();
  }

  int ScrollPane::getMaxScrollH() const
  {
    return this->horizontalScrollBar.getMaxValue();
  }

  int ScrollPane::getMaxScrollV() const
  {
    return this->verticalScrollBar.getMaxValue();
  }

  int ScrollPane::getMaxUsefulScrollV() const
  {
    return this->verticalScrollBar.getMaxUsefulValue();
  }

  bool ScrollPane::mouseWheelDown(const MouseEvent& mouseEvent)
  {
    if (mouseEvent.shift() && this->horizontalScrollBar.isVisible())
      return this->horizontalScrollBar.mouseWheelLeft(mouseEvent);
    if (this->verticalScrollBar.isVisible())
      return this->verticalScrollBar.mouseWheelDown(mouseEvent);
    return super::mouseWheelDown(mouseEvent);
  }

  bool ScrollPane::mouseWheelUp(const MouseEvent& mouseEvent)
  {
    if (mouseEvent.shift() && this->horizontalScrollBar.isVisible())
      return this->horizontalScrollBar.mouseWheelRight(mouseEvent);
    if (this->verticalScrollBar.isVisible())
      return this->verticalScrollBar.mouseWheelUp(mouseEvent);
    return super::mouseWheelUp(mouseEvent);
  }

  bool ScrollPane::mouseWheelLeft(const MouseEvent& mouseEvent)
  {
    if (this->horizontalScrollBar.isVisible())
      return this->horizontalScrollBar.mouseWheelLeft(mouseEvent);
    return super::mouseWheelLeft(mouseEvent);
  }

  bool ScrollPane::mouseWheelRight(const MouseEvent& mouseEvent)
  {
    if (this->horizontalScrollBar.isVisible())
      return this->horizontalScrollBar.mouseWheelRight(mouseEvent);
    return super::mouseWheelRight(mouseEvent);
  }

  void ScrollPane::adjustScrollBarRanges()
  {
    int extraH = 0;
    int extraV = 0;

    if (this->horizontalScrollBar.isVisible())
      extraH += this->horizontalScrollBar.getHeight();

    if (this->verticalScrollBar.isVisible())
      extraV += this->verticalScrollBar.getWidth();

    // set vertical value
    this->verticalScrollBar.setRangeFromPage(this->getHeight() - extraH, this->getContentsHeight() + this->getVerticalPaddings());

    // set horizontal value
    this->horizontalScrollBar.setRangeFromPage(this->getWidth() - extraV, this->getContentsWidth() + this->getHorizontalPaddings());
  }

  void ScrollPane::checkScrollPositions()
  {
    const bool hScrollVisible = this->horizontalScrollBar.isVisible();
    const bool vScrollVisible = this->verticalScrollBar.isVisible();
    this->rememberedVerticalPosition = std::max(this->rememberedVerticalPosition, this->verticalScrollBar.getValue());
    this->rememberedHorizontalPosition = std::max(this->rememberedHorizontalPosition, this->horizontalScrollBar.getValue());
    if (this->centerOrdered)
      this->centerInternal();
    if (!hScrollVisible && !vScrollVisible)
      this->scrollToInternal(0, 0);
    else if (!hScrollVisible)
      this->scrollToInternal(0, this->verticalScrollBar.getValue());
    else if (!vScrollVisible)
      this->scrollToInternal(this->horizontalScrollBar.getValue(), 0);

    if (vScrollVisible && this->rememberedVerticalPosition != 0)
    {
      this->verticalScrollBar.setValue(this->rememberedVerticalPosition);
      this->scrollToInternal(this->horizontalScrollBar.getValue(), this->verticalScrollBar.getValue());
    }

    if (hScrollVisible && this->rememberedHorizontalPosition != 0)
    {
      this->horizontalScrollBar.setValue(this->rememberedHorizontalPosition);
      this->scrollToInternal(this->horizontalScrollBar.getValue(), this->verticalScrollBar.getValue());
    }
  }

  void ScrollPane::setWheelScrollRate(int rate)
  {
    this->verticalScrollBar.setMouseWheelAmount(rate);
    this->horizontalScrollBar.setMouseWheelAmount(rate);
  }

  int ScrollPane::getWheelScrollRate() const
  {
    return this->verticalScrollBar.getMouseWheelAmount();
  }

  void ScrollPane::setHKeyScrollRate(int rate)
  {
    this->hKeyScrollRate = rate;
  }

  int ScrollPane::getHKeyScrollRate() const
  {
    return this->hKeyScrollRate;
  }

  void ScrollPane::setVKeyScrollRate(int rate)
  {
    this->vKeyScrollRate = rate;
  }

  int ScrollPane::getVKeyScrollRate() const
  {
    return this->vKeyScrollRate;
  }

  void ScrollPane::keyAction(ExtendedKeyEnum key, bool shift)
  {
    (void)(shift);
    switch (key)
    {
      case EXT_KEY_UP: this->scrollToV(this->verticalScrollBar.getValue() - getVKeyScrollRate()); break;
      case EXT_KEY_DOWN: this->scrollToV(this->verticalScrollBar.getValue() + getVKeyScrollRate()); break;
      case EXT_KEY_LEFT: this->scrollToH(this->horizontalScrollBar.getValue() - getHKeyScrollRate()); break;
      case EXT_KEY_RIGHT: this->scrollToH(this->horizontalScrollBar.getValue() + getHKeyScrollRate()); break;
      case EXT_KEY_PAGE_DOWN: this->scrollToV(this->verticalScrollBar.getValue() + this->verticalScrollBar.getLargeAmount()); break;
      case EXT_KEY_PAGE_UP: this->scrollToV(this->verticalScrollBar.getValue() - this->verticalScrollBar.getLargeAmount()); break;
      default: break;
    }
  }

  void ScrollPane::updatecontentFlowPosition()
  {
    this->contentFlow.setLocation(-this->horizontalScrollBar.getValue(), -this->verticalScrollBar.getValue());
  }

  void ScrollPane::recalculateClippingRect() const
  {
     if (this->clippingRectangle)
      return;
    // Unlike every other widget, scroll pane clips its content, so the clipping widget is always equal to its size.
    this->clippingRectangle = this->getSizeRectangle();
    if (this->verticalScrollBar.isVisible() && this->style.getScrollbarsGoOutside())
      this->clippingRectangle->width += this->verticalScrollBar.getWidth();
  }

  void ScrollPane::setHMinThumbSize(int size)
  {
    this->horizontalScrollBar.setMinThumbSize(size);
  }

  int ScrollPane::getHMinThumbSize() const
  {
    return this->horizontalScrollBar.getMinThumbSize();
  }

  void ScrollPane::setVMinThumbSize(int size)
  {
    this->verticalScrollBar.setMinThumbSize(size);
  }

  int ScrollPane::getVMinThumbSize() const
  {
    return this->verticalScrollBar.getMinThumbSize();
  }

  void ScrollPane::reapplySubStyles()
  {
    this->contentFlow.style.setParent(this->style.getVerticalFlowStyle());
    this->horizontalScrollBar.style.setParent(this->style.getHorizontalScrollBarStyle());
    this->verticalScrollBar.style.setParent(this->style.getVerticalScrollBarStyle());
  }

  void ScrollPane::postDisplaySizeChanged()
  {
    Widget::postDisplaySizeChanged();
    this->updateScrollBars();
  }

  bool ScrollPane::isPointVisible(const agui::Point& point) const
  {
    auto& contentLocation = this->contentFlow.getLocation();
    return point.y + contentLocation.y >= 0 &&
           point.y + contentLocation.y <= this->getHeight() &&
           point.x + contentLocation.x >= 0 &&
           point.x + contentLocation.x <= this->getWidth();
  }

  bool ScrollPane::isAreaVisible(const agui::Rectangle& rectangle) const
  {
    return this->isPointVisible(rectangle.getLeftTop()) ||
           this->isPointVisible(rectangle.getRightBottom());
  }

  void ScrollPane::flagAllChildrenForDestruction()
  {
    Widget::flagAllChildrenForDestruction();
    this->contentFlow.flagAllChildrenForDestruction();
  }

  void ScrollPane::resizeWidthToContents()
  {
    int vscroll = 0;
    if (this->shouldReserveSpaceForVerticalScrollbar())
      vscroll = this->verticalScrollBar.getWidth();

    int newWidth = this->getHorizontalPaddings() + this->getContentsWidth() + vscroll;
    if (newWidth != this->getWidth())
      this->setSize(newWidth, this->getHeight());
  }

  void ScrollPane::resizeHeightToContents()
  {
    int hscroll = 0;
    if (this->shouldReserveSpaceForHorizontalScrollbar())
      hscroll = this->horizontalScrollBar.getHeight();

    int newHeight = this->getVerticalPaddings() + this->getContentsHeight() + hscroll;
    if (this->style.isVerticallyStretchable() == StretchRule::On)
      newHeight = this->style.getMinimalHeight();
    if (newHeight != this->getHeight())
      this->setSize(this->getWidth(), newHeight);
  }

  void ScrollPane::resizeToContents()
  {
    // reset the scrollbars visibility before resizing to contents.
    {
      this->horizontalScrollBar.setValue(0);
      this->horizontalScrollBar.setVisibleSilent(false);
      this->verticalScrollBar.setValue(0);
      this->verticalScrollBar.setVisibleSilent(false);
    }

    bool horizontallyExpand = this->getHScrollPolicy() == ScrollPolicy::Never || this->style.isHorizontallyStretchable() != StretchRule::On;
    bool verticallyExpand = this->style.isVerticallyStretchable() != StretchRule::On;
    int width = (horizontallyExpand && this->contentFlow.isVisible()) ? this->contentFlow.getWidth() : this->style.getMinimalWidth();
    int height = (verticallyExpand && this->contentFlow.isVisible()) ? this->contentFlow.getHeight() : this->style.getMinimalHeight();
    this->setContentSizeInternal(width, height);
    this->checkScrollPolicy();
    if (horizontallyExpand)
      this->resizeWidthToContents();
    if (verticallyExpand)
      this->resizeHeightToContents();
    this->adjustScrollBarRanges();
    this->checkScrollPositions();
  }

  void ScrollPane::clear()
  {
    this->contentFlow.clear();
    Widget::clear();
  }

  void ScrollPane::keepVerticallyVisibleInScrollPane(int verticalPosition, int margin)
  {
    if (verticalPosition - margin < 0)
      this->scrollToV(verticalPosition - margin - this->contentFlow.getLocation().y);
    else if (verticalPosition + margin > this->getHeight())
      this->scrollToV(verticalPosition - this->contentFlow.getLocation().y - this->getHeight() + margin);
    // no super call - we end the propagation
  }

  int ScrollPane::getVerticalScrollPosition()
  {
    return this->verticalScrollBar.getValue();
  }

  void ScrollPane::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    graphicsContext->setClippingRectangleToTheLastForced();
    {
      Rectangle rectangleToClip(Point(0, 0), this->getSize());
      if (this->style.getScrollbarsGoOutside() && this->verticalScrollBar.isVisible())
        rectangleToClip.width += this->verticalScrollBar.getWidth();
      graphicsContext->pushClippingRect(this, rectangleToClip);
    }
    this->paintBackground(PaintEvent(enabled, graphicsContext), absolutePosition);

    const int leftPadding = this->getLeftPadding();
    const int topPadding = this->getTopPadding();

    if (this->horizontalScrollBar.isVisible())
      this->horizontalScrollBar.recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, this->horizontalScrollBar.getLocation(), leftPadding, topPadding));
    if (this->verticalScrollBar.isVisible())
      this->verticalScrollBar.recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, this->verticalScrollBar.getLocation(), leftPadding, topPadding));

    //rootAbsolutePosition.x += this->getLeftPadding();
    //rootAbsolutePosition.y += this->getTopPadding();
    graphicsContext->setOffset(absolutePosition);

    //If the scroll pane is not activated and the flag is set in the style, we allow contentFlow to draw outside of this ScrollPane.
    //This is useful when we have an outer draw style inside of contentFlow that we want to be visible
    //The solution is not ideal but this can't be solved using extra_margin/padding_when_activated as it causes many layouting issues.
    const bool isActiveDrawing = this->style.getAlwaysDrawBorders() || this->isActivated();
    graphicsContext->pushClippingRect(this, Rectangle(Point(0, 0), this->getSizeForContent(false, false)), isActiveDrawing || !this->style.shouldNotForceClippingRectForContents());
    graphicsContext->setOffset(Point(absolutePosition.x + leftPadding, absolutePosition.y + topPadding));
    this->paintComponent(PaintEvent(enabled, graphicsContext), absolutePosition);

    if (this->contentFlow.isVisible())
      this->contentFlow.recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, this->contentFlow.getLocation(), leftPadding, topPadding));
    graphicsContext->popClippingRect();
    if (isActiveDrawing)
    {
      graphicsContext->setOffset(absolutePosition);
      if (graphicsContext->shadowView)
        this->style.getGraphicalSet()->shadow.draw(PaintEvent(enabled, graphicsContext),
                                                   Rectangle(Point(0, 0), this->getSizeForContent(false, false)),
                                                   absolutePosition);
    }
    graphicsContext->popClippingRect();
  }

  void ScrollPane::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    (void)forceInFrame;
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, Rectangle(Point(0, 0), this->getSize()), this->isActivated()))
    {
      if (includeThis)
        this->paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition);

      const int leftPadding = this->getLeftPadding();
      const int topPadding = this->getTopPadding();

      for (Widget* widget : this->getPrivateChildren())
        if (widget->isVisible() && widget->shouldRender())
          widget->recursivePaintShadows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));

      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintShadows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });
    }
    graphicsContext->popClippingRect();
  }

  void ScrollPane::recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    (void)forceInFrame;
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);

    const int leftPadding = this->getLeftPadding();
    const int topPadding = this->getTopPadding();

    if (graphicsContext->pushClippingRect(this, Rectangle(Point(0, 0), this->getSize()), this->isActivated()))
    {
      if (includeThis)
        this->paintBackgroundGlow(PaintEvent(enabled, graphicsContext), absolutePosition);
      if (this->contentFlow.isVisible() && this->contentFlow.shouldRender())
        this->contentFlow.recursivePaintGlows(enabled, graphicsContext, Point(absolutePosition, this->contentFlow.getLocation(), leftPadding, topPadding));

      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintGlows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });
    }
    graphicsContext->popClippingRect();

    // we draw the scroll bar glows without the limitation of clipping rect, so they are not cropped.
    for (Widget* widget : this->getPrivateChildren())
      if (widget != &this->contentFlow && widget->isVisible() && widget->shouldRender())
        widget->recursivePaintGlows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));
  }

  void ScrollPane::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (this->style.getAlwaysDrawBorders() || this->isActivated())
      this->style.getGraphicalSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void ScrollPane::paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getBackgroundGraphcialSet()->base.draw(paintEvent, this->contentFlow.getRelativeRectangle(), absolutePosition);
  }

  agui::Dimension ScrollPane::getSizeForContent(bool includePaddings, bool includeReservedSpaceForScrollBars) const
  {
    int horizontalPaddings = includePaddings ? this->getHorizontalPaddings() : 0;
    int verticalPaddings = includePaddings ? this->getVerticalPaddings() : 0;
    bool includeVerticalScrollbar = this->verticalScrollBar.isVisible() && !this->style.getScrollbarsGoOutside() ||
                                    (includeReservedSpaceForScrollBars && this->vScrollPolicy == ScrollPolicy::AutoAndReserveSpace);
    bool includeHorizontalScrollbar = this->horizontalScrollBar.isVisible() ||
                                      (includeReservedSpaceForScrollBars && this->hScrollPolicy == ScrollPolicy::AutoAndReserveSpace);
    int verticalScrollbarWidth = includeVerticalScrollbar ? this->verticalScrollBar.getWidth() : 0;
    int horizontalScrollbarHeight = includeHorizontalScrollbar ? this->horizontalScrollBar.getHeight() : 0;

    return Dimension(this->getWidth() - verticalScrollbarWidth - horizontalPaddings,
                     this->getHeight() - horizontalScrollbarHeight - verticalPaddings);
  }

  void ScrollPane::center()
  {
    this->centerOrdered = true;
    this->centerInternal();
  }


  bool ScrollPane::isVerticalBarActivated() const
  {
    return this->verticalScrollBar.isVisible() || this->vScrollPolicy == ScrollPolicy::AutoAndReserveSpace;
  }

  bool ScrollPane::isHorizontalBarActivated() const
  {
    return this->horizontalScrollBar.isVisible() ||  this->hScrollPolicy == ScrollPolicy::AutoAndReserveSpace;
  }

  bool ScrollPane::isActivated() const
  {
    return this->isVerticalBarActivated() || this->isHorizontalBarActivated() || // border even when scrollbar not visible
           this->style.getAlwaysDrawBorders();
  }

  const ElementImageSet* ScrollPane::getBorderImageSet() const
  {
    if (this->isActivated())
      return this->style.getGraphicalSet();
    return nullptr;
  }

  int ScrollPane::getTopPadding() const
  {
    return (this->isActivated() ? this->style.getExtraTopPaddingWhenActivated() : 0) +
           this->style.getTopPadding() +
           this->getTopBorder();
  }

  int ScrollPane::getBottomPadding() const
  {
    return (this->isActivated() ? this->style.getExtraBottomPaddingWhenActivated() : 0) +
           this->style.getBottomPadding() +
           this->getBottomBorder();
  }

  int ScrollPane::getLeftPadding() const
  {
    return (this->isActivated() ? this->style.getExtraLeftPaddingWhenActivated() : 0) +
           this->style.getLeftPadding() +
           this->getLeftBorder();
  }

  int ScrollPane::getRightPadding() const
  {
    int result = this->style.getRightPadding() + this->getRightBorder();
    if (this->isActivated())
      result += this->style.getExtraRightPaddingWhenActivated();
    return result;
  }

  int ScrollPane::getTopMargin() const
  {
    return (this->isActivated() ? this->style.getExtraTopMarginWhenActivated() : 0) +
           super::getTopMargin();
  }

  int ScrollPane::getBottomMargin() const
  {
    return (this->isActivated() ? this->style.getExtraBottomMarginWhenActivated() : 0) +
           super::getBottomMargin();
  }

  int ScrollPane::getLeftMargin() const
  {
    return (this->isActivated() ? this->style.getExtraLeftMarginWhenActivated() : 0) +
           super::getLeftMargin();
  }

  int ScrollPane::getRightMargin() const
  {
    int result = super::getRightMargin();
    if (this->isActivated())
      result += this->style.getExtraRightMarginWhenActivated();
    return result;
  }

  void ScrollPane::afterResizeIsSettledAction()
  {
    if (this->scrollToWidgetOnNextResize)
    {
      this->scrollToMakeWidgetVisibleInternal(*this->scrollToWidgetOnNextResize, this->scrollModeOfScrollToOnNextResize);
      this->scrollToWidgetOnNextResize.clear();
    }
    if (this->scrollToAreaOnNextResize)
    {
      this->scrollToMakePositionVisibleInternal(*this->scrollToAreaOnNextResize, this->scrollModeOfScrollToOnNextResize);
      this->scrollToAreaOnNextResize = std::nullopt;
    }
  }

  void ScrollPane::centerInternal()
  {
    if (this->horizontalScrollBar.isVisible())
      this->horizontalScrollBar.scrollToCenter();
    if (this->verticalScrollBar.isVisible())
      this->verticalScrollBar.scrollToCenter();
    this->updatecontentFlowPosition();
    this->rememberedHorizontalPosition = 0;
    this->rememberedVerticalPosition = 0;
  }

  void ScrollPane::scrollToMakeWidgetVisibleInternal(Widget* widget, ScrollMode scrollMode)
  {
    agui::Rectangle absolutWidgetRectangle = widget->getAbsoluteRectangle();
    this->scrollToMakePositionVisibleInternal(absolutWidgetRectangle, scrollMode);
  }

  void ScrollPane::scrollToMakePositionVisibleInternal(Rectangle rectangle, ScrollMode scrollMode)
  {
    const agui::Point thisAbsolute = this->getAbsolutePosition();

    int32_t relativeYTop = rectangle.getTop() - thisAbsolute.y;
    int32_t relativeYBottom = rectangle.getBottom() - thisAbsolute.y;
    int32_t relativeXLeft = rectangle.getLeft() - thisAbsolute.x;
    int32_t relativeXRight = rectangle.getRight() - thisAbsolute.x;
    if (scrollMode == ScrollMode::TopThird)
    {
      this->scrollTo(this->getCurrentScrollX() + relativeXLeft - this->getWidth() / 3,
                     this->getCurrentScrollY() + relativeYTop - this->getHeight() / 3);
      return;
    }

    // ScrollMode InView
    int targetX = this->getCurrentScrollX();
    int targetY = this->getCurrentScrollY();

    if (relativeXLeft < 0)
      targetX = this->getCurrentScrollX() + relativeXLeft;
    else if (relativeXRight > this->getContentWidth())
      targetX = this->getCurrentScrollX() + relativeXRight - this->getContentWidth();

    if (relativeYTop < 0)
      targetY = this->getCurrentScrollY() + relativeYTop;
    else if (relativeYBottom > this->getContentHeight())
      targetY = this->getCurrentScrollY() + relativeYBottom - this->getContentHeight();

    if (targetX != this->getCurrentScrollX() || targetY != this->getCurrentScrollY())
      this->scrollTo(targetX, targetY);
  }

  bool ScrollPane::shouldReserveSpaceForVerticalScrollbar() const
  {
   if (this->style.getScrollbarsGoOutside())
      return false;
    if (this->verticalScrollBar.isVisible())
      return true;
    if (this->vScrollPolicy == ScrollPolicy::AutoAndReserveSpace)
      return true;
    return false;
  }

  bool ScrollPane::shouldReserveSpaceForHorizontalScrollbar() const
  {
    return (this->horizontalScrollBar.isVisible() || this->hScrollPolicy == ScrollPolicy::AutoAndReserveSpace);
  }
}

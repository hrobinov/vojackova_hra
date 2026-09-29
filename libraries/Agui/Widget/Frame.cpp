#include "Agui/DrawCulledChildren.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/TopContainer.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/Frame.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include <Agui/Math.hpp>
#include <algorithm>
#include <cassert>

namespace agui
{
  FrameStyle Frame::defaultStyle;

  Frame::Frame(GuiDirection direction, const agui::FrameStyle* parentStyle, const std::string& title)
    : style(this, parentStyle == nullptr ? &agui::Frame::defaultStyle : parentStyle)
    , layout(direction == GuiDirection::Horizontal ?
             static_cast<agui::Layout*>(new agui::HorizontalFlow(parentStyle == nullptr
                                                                 ? &agui::HorizontalFlow::defaultStyle
                                                                 : parentStyle->getHorizontalFlowStyle())) :
             static_cast<agui::Layout*>(new agui::VerticalFlow(parentStyle == nullptr
                                                               ? &agui::VerticalFlow::defaultStyle
                                                               : parentStyle->getVerticalFlowStyle())))
    , direction(direction)
    , headerFlow(this->style.getHeaderFlowStyle())
    , filler(this->style.getHeaderFillerStyle())
    , title(this->style.getTitleStyle())
  {
    this->layout->autoDestructWhenRemovedFromParent();
    this->addPrivateChild(this->layout);

    this->title.style.setHorizontallySquashable(true);
    this->title.style.setVerticallyStretchable(true);
    this->title.setText(title);
    this->headerFlow << this->title;
    this->headerFlow << this->filler;

    this->updateHeaderFillerVisibility();

    Widget::add(&this->headerFlow);
    this->headerFlow.style.setVerticallyStretchable(agui::StretchRule::Off);
    this->updateHeaderFlowVisibility();
    this->shrinkInReactionToSetSize();
  }

  Frame::Frame(GuiDirection direction, const FrameStyle* parentStyle)
    : Frame(direction, parentStyle, "")
  {}

  Frame::Frame(GuiDirection direction, const std::string& title)
    : Frame(direction, nullptr, title)
  {}

  agui::Widget& Frame::dontShrinkInReactionToSetSize()
  {
    super::dontShrinkInReactionToSetSize();
    this->layout->dontShrinkInReactionToSetSize();
    return *this;
  }

  void Frame::reapplySubStyles()
  {
    this->layout->getStyle()->setParent(this->direction == GuiDirection::Horizontal ?
                                        static_cast<const Style*>(this->style.getHorizontalFlowStyle()) :
                                        static_cast<const Style*>(this->style.getVerticalFlowStyle()));
    this->headerFlow.style.setParent(this->style.getHeaderFlowStyle());
    this->filler.style.setParent(this->style.getHeaderFillerStyle());
    this->title.style.setParent(this->style.getTitleStyle());
    this->updateHeaderFillerVisibility();
  }

  Frame& Frame::operator<<(const Pusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->style.setHorizontallyStretchable();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  void Frame::add(Widget* widget)
  {
    this->layout->add(widget);
  }

  void Frame::addBefore(Widget* toAdd, const Widget* before)
  {
    auto it = std::find(this->layout->begin(), this->layout->end(), before);
    assert(it != this->layout->end());
    this->insert(toAdd, std::distance(this->layout->getChildren().data(), &*it));
  }

  void Frame::insert(Widget* widget, uint32_t index)
  {
    this->layout->insert(widget, index);
  }

  void Frame::addFront(Widget* widget)
  {
    this->layout->addFront(widget);
  }

  void Frame::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    if (layout->containsChildWidget(widget))
      layout->remove(widget, keepWidgetAlive);
    else
      Widget::remove(widget, keepWidgetAlive);
  }

  void Frame::swapChildren(size_t index1, size_t index2)
  {
    this->layout->swapChildren(index1, index2);
  }

  bool Frame::empty() const
  {
    return this->layout->empty();
  }

  void Frame::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, this->getSizeRectangle()))
    {
      this->paint(PaintEvent(enabled, graphicsContext), absolutePosition);
      if (this->isInset())
        graphicsContext->pushClippingRect(this, this->getSizeRectangle(), true, &absolutePosition);

      // shadow of this frame is drawn later to draw over content in the case of inset
      this->recursivePaintShadows(enabled, graphicsContext, absolutePosition, true, false /* includeThis*/);

      const int leftPadding = this->getLeftPadding();
      const int topPadding = this->getTopPadding();

      for (Widget* widget : this->getPrivateChildren())
        if (widget->isVisible() && widget->shouldRender())
          widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));

      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });

      graphicsContext->setOffset(absolutePosition);
      this->paintBackgroundShadow(PaintEvent(enabled, graphicsContext), absolutePosition);
      if (this->isInset())
        graphicsContext->popClippingRect();
    }
    graphicsContext->popClippingRect();
    this->recursivePaintGlows(enabled, graphicsContext, absolutePosition, true);
  }

  void Frame::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (forceInFrame)
      super::recursivePaintShadows(enabled, graphicsContext, absolutePosition, forceInFrame, includeThis);
  }

  void Frame::recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (forceInFrame)
      super::recursivePaintGlows(enabled, graphicsContext, absolutePosition, forceInFrame, includeThis);
  }

  int Frame::getTopPartHeight() const
  {
    return this->headerFlow.isVisible() ? this->headerFlow.getHeight() : 0;
  }

  void Frame::updateFillerRightMargin()
  {
    if (!this->headerFlow.getChildren().empty() &&
        this->headerFlow.getChildren().back() != &this->filler)
      this->filler.style.setRightMargin(Gui::scale * Frame::EXTRA_RIGHT_MARGIN_WHEN_SOMETHING_IS_NEXT_TO_FILLER);
  }

  int Frame::requiredTopPartWidth()
  {
    return this->headerFlow.getWidth();
  }

  void Frame::updateHeaderFlowSizeAndLocation()
  {
    int headerHeight = 0;
    if (this->title.getTextLength() > 0)
      headerHeight = this->title.style.getFont()->getLineHeight() + this->title.style.getTopPadding() + this->title.style.getBottomPadding();
    else
      headerHeight = this->headerFlow.getHeight();
    this->headerFlow.setSize(this->getContentWidth(), headerHeight);
    this->headerFlow.setLocation(0, 0);
  }

  void Frame::updateHeaderFlowVisibility()
  {
    this->headerFlow.setVisible(!this->title.getText().empty() || this->headerFlow.getChildCount() > 2);
  }

  void Frame::resizeContainer(SetSizeInfo setSizeInfo)
  {
    int topPartHeight = this->getTopPartHeight();
    this->layout->setLocation(0, topPartHeight);
    int oldContentHeight = this->getContentSize().height - topPartHeight;
    int oldContentWidth = this->getContentSize().width;
    this->layout->setSize(oldContentWidth, oldContentHeight, setSizeInfo);

    if (oldContentHeight > layout->getHeight() &&
        (this->canShrink(setSizeInfo.reactionToSetSize) || setSizeInfo.vertical == Change::Nothing))  // content is smaller than required, make this also smaller, but not smaller than natural height
      super::setSize(this->getWidth(), Math::max(layout->getHeight() + topPartHeight + this->getVerticalPaddings(), this->style.getNaturalHeight()), setSizeInfo);
    else if (oldContentHeight < layout->getHeight())
      super::setSize(this->getWidth(), layout->getHeight() + topPartHeight + this->getVerticalPaddings(), setSizeInfo); // result size is bigger, just enlarge this

    if (oldContentWidth > layout->getWidth() && this->canShrink(setSizeInfo.reactionToSetSize)) // content is smaller than required, make this also smaller, but not smaller than natural width
      super::setSize(Math::max(layout->getWidth() + this->getHorizontalPaddings(), this->style.getNaturalWidth()), this->getHeight(), setSizeInfo);
    else if (oldContentWidth < layout->getWidth())
      super::setSize(layout->getWidth() + this->getHorizontalPaddings(), this->getHeight(), setSizeInfo); // result width is bigger, just enlarge this
  }

  void Frame::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (!this->style.usesBorder() && paintEvent.graphics()->shadowView)
      this->style.getGraphicalSet()->shadow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void Frame::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getGraphicalSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
    if (this->style.getHeaderBackground())
    {
      int topSize = this->getTopPadding() - this->getTopBorder() + this->getTopPartHeight();
      this->style.getHeaderBackground()->base.draw(paintEvent,
                                                   Rectangle(this->getLeftBorder(),
                                                             this->getTopBorder(),
                                                             this->getWidth() - this->getRightBorder() - this->getLeftBorder(),
                                                             topSize), absolutePosition);
      int footerSize = this->getBottomPadding() - this->getBottomBorder();
      this->style.getHeaderBackground()->base.draw(paintEvent,
                                                   Rectangle(this->getLeftBorder(),
                                                             this->getHeight() - footerSize - this->getBottomBorder(),
                                                             this->getWidth() - this->getRightBorder() - this->getLeftBorder(),
                                                             footerSize), absolutePosition);
      int leftSize = this->getLeftPadding() - this->getLeftBorder();
      this->style.getHeaderBackground()->base.draw(paintEvent,
                                                   Rectangle(this->getLeftBorder(),
                                                             this->getTopBorder(),
                                                             leftSize,
                                                             this->getHeight() - this->getBottomBorder() - this->getTopBorder()), absolutePosition);
      int rightSize = this->getRightPadding() - this->getRightBorder();
      this->style.getHeaderBackground()->base.draw(paintEvent,
                                                   Rectangle(this->getWidth() - rightSize - this->getRightBorder(),
                                                             this->getTopBorder(),
                                                             rightSize,
                                                             this->getHeight() - this->getBottomBorder() - this->getTopBorder()), absolutePosition);
    }

    if (!this->style.usesBorder() && !paintEvent.graphics()->debugView)
      this->paintBackgroundGlow(paintEvent, absolutePosition);

    if (this->style.usesBorder())
      this->style.getBorder()->drawBorder(paintEvent, Point(0, 0), Dimension(this->getSizeRectangle().width, this->getSizeRectangle().height));

    paintEvent.graphics()->pushClippingRect(this, this->getSizeRectangle(), true);
    if (const ElementImageSet* background = this->style.getBackgroundGraphicalSet())
      background->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
    paintEvent.graphics()->popClippingRect();
  }

  void Frame::paintBackgroundGlow(const PaintEvent& paintEvent, const Point& absolutePosition)
  {
    if (paintEvent.graphics()->glowView)
      this->style.getGraphicalSet()->glow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  Widget* Frame::getContentHolder()
  {
    return this->layout;
  }

  void Frame::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    super::setSize(width, height, setSizeInfo);
    this->resizeContainer(setSizeInfo);
    this->updateHeaderFlowSizeAndLocation();
  }

  void Frame::flagAllChildrenForDestruction()
  {
    Widget::flagAllChildrenForDestruction();
    this->getContentHolder()->flagAllChildrenForDestruction();
  }

  int Frame::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;
    int maximumSquashSizeLimit = std::max(0, this->getHeight() - this->style.getMinimalHeight());
    return std::min(maximumSquashSizeLimit,  this->layout->maximumVerticalSquashSize());
  }

  int Frame::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;
    int maximumSquashSizeLimit = std::max(0, this->getWidth() - this->style.getMinimalWidth());
    return std::min(maximumSquashSizeLimit, this->layout->maximumHorizontalSquashSize());
  }

  void Frame::displaySizeChanged()
  {
    super::displaySizeChanged();
    this->updateFillerRightMargin();
  }

  const ElementImageSet* Frame::getBorderImageSet() const
  {
    return this->style.getGraphicalSet();
  }

  void Frame::resizeToContents()
  {
    int topPartHeight = this->getTopPartHeight();
    this->layout->setLocation(0, topPartHeight);

    if (this->layout->isVisible())
      this->setContentSizeInternal(std::max(std::max(this->layout->getWidth(), this->requiredTopPartWidth()), this->style.getNaturalWidth() - this->getHorizontalPaddings()),
                                   std::max(this->layout->getHeight() + topPartHeight, this->style.getNaturalHeight() - this->getVerticalPaddings()));
    else
      this->setContentSizeInternal(std::max(this->requiredTopPartWidth(), this->style.getNaturalWidth() - this->getHorizontalPaddings()),
                                   std::max(topPartHeight, this->style.getNaturalHeight() - this->getVerticalPaddings()));
    Dimension contentSize = this->getContentSize();
    contentSize.height -= topPartHeight;

    // limited by the style max width/height
    if (contentSize != this->layout->getSize() && this->layout->isVisible())
    {
      int originalWidth = this->layout->getWidth();
      int originalHeight = this->layout->getHeight();
      SetSizeInfo setSizeInfo(Change::Nothing, Change::Nothing);
      int targetWidth = this->layout->getWidth();
      int targetHeight = this->layout->getHeight();

      if (contentSize.width > this->layout->getWidth())
      {
        if (this->layout->isHorizontallyStretchable())
        {
          setSizeInfo.horizontal = Change::Stretching;
          targetWidth = contentSize.width;
        }
      }
      else if (contentSize.width < this->layout->getWidth())
      {
        setSizeInfo.horizontal = Change::Squashing;
        targetWidth = contentSize.width;
      }

      if (contentSize.height > this->layout->getHeight())
      {
        if (this->layout->isVerticallyStretchable())
        {
          setSizeInfo.vertical = Change::Stretching;
          targetHeight = contentSize.height;
        }
      }
      else if (contentSize.height < this->layout->getHeight())
      {
        setSizeInfo.vertical = Change::Squashing;
        targetHeight = contentSize.height;
      }
      if (this->layout->getSize() != Dimension(targetWidth, targetHeight))
        this->layout->setSize(targetWidth, targetHeight, setSizeInfo);

      if (this->layout->getWidth() != originalWidth ||
          this->layout->getHeight() != originalHeight)
      {
        int desiredWidth = std::max(this->layout->getWidth(), this->style.getNaturalWidth() - this->getHorizontalPaddings());
        int desiredHeight = std::max(this->layout->getHeight() + topPartHeight, this->style.getNaturalHeight() - this->getVerticalPaddings());
        this->setContentSizeInternal(std::max(desiredWidth, this->requiredTopPartWidth()), desiredHeight);
      }
      int x = 0, y = this->getTopPartHeight();

      agui::HorizontalAlign horizontalAlign = this->style.getHorizontalAlign();
      if (horizontalAlign != agui::HorizontalAlign::Left)
      {
        int extraHorizontalSize = this->getContentSize().width - this->getContentHolder()->getWidth();
        if (extraHorizontalSize > 0)
          if (horizontalAlign == agui::HorizontalAlign::Right)
            x = this->getContentSize().width - this->getContentHolder()->getWidth();
          else if (horizontalAlign == agui::HorizontalAlign::Center)
            x = (this->getContentSize().width - this->getContentHolder()->getWidth()) / 2;
      }

      agui::VerticalAlign verticalAlign = this->style.getVerticalAlign();
      if (verticalAlign != agui::VerticalAlign::Top)
      {
        int extraVerticalSize = this->getContentSize().height - this->getContentHolder()->getHeight() - y;
        if (extraVerticalSize > 0)
          if (verticalAlign == agui::VerticalAlign::Bottom)
            y = this->getContentSize().height - this->getContentHolder()->getHeight();
          else if (verticalAlign == agui::VerticalAlign::Center)
            y = (this->getContentSize().height - this->getContentHolder()->getHeight()) / 2;
      }
      if (this->getContentHolder()->getLocation() != agui::Point(x, y))
        this->getContentHolder()->setLocation(x, y);
    }
    this->updateHeaderFlowSizeAndLocation();
  }

  int Frame::getTopBorder() const
  {
    int result = 0;
    if (const ElementImageSet* elementImageSet = this->style.getGraphicalSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        result += this->getBorderImageSet()->getTopBorder();
    if (const BorderImageSet* borderImageSet = this->style.getBorder())
      if (borderImageSet->isSet)
        result += borderImageSet->lineWidth;
    return result;
  }

  int Frame::getRightBorder() const
  {
    int result = 0;
    if (const ElementImageSet* elementImageSet = this->style.getGraphicalSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        result += this->getBorderImageSet()->getRightBorder();
    if (const BorderImageSet* borderImageSet = this->style.getBorder())
      if (borderImageSet->isSet)
        result += borderImageSet->lineWidth;
    return result;
  }

  int Frame::getBottomBorder() const
  {
    int result = 0;
    if (const ElementImageSet* elementImageSet = this->style.getGraphicalSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        result += this->getBorderImageSet()->getBottomBorder();
    if (const BorderImageSet* borderImageSet = this->style.getBorder())
      if (borderImageSet->isSet)
        result += borderImageSet->lineWidth;
    return result;
  }

  int Frame::getLeftBorder() const
  {
    int result = 0;
    if (const ElementImageSet* elementImageSet = this->style.getGraphicalSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        result += this->getBorderImageSet()->getLeftBorder();
    if (const BorderImageSet* borderImageSet = this->style.getBorder())
      if (borderImageSet->isSet)
        result += borderImageSet->lineWidth;
    return result;
  }

  TransparentValue Frame::isTransparent() const
  {
    return (this->style.getGraphicalSet()->base.type == ElementImageSet::Layer::Type::None || forceTransparent) ? TransparentValue::DependsOnChildren : TransparentValue::No;
  }

  void Frame::clear()
  {
    this->layout->clear();
  }

  void Frame::addToHeaderFlow(Widget* widget)
  {
    this->headerFlow << widget;
    this->updateHeaderFlowVisibility();
    this->updateFillerRightMargin();
  }

  void Frame::addToHeaderFlow(Pusher)
  {
    this->headerFlow << agui::pusher;
  }

  void Frame::addToHeaderFlowLeft(Widget* widget)
  {
    this->headerFlow.insert(widget, 1); // behind title label
    this->updateHeaderFlowVisibility();
  }

  void Frame::addToHeaderFlowBeforeLast(Widget* widget)
  {
    this->headerFlow.insert(widget, this->headerFlow.getChildCount() - 1);
    this->updateHeaderFlowVisibility();
  }

  void Frame::updateHeaderFillerVisibility()
  {
    this->filler.setVisible(this->style.getUseHeaderFiller());
  }

  void Frame::setText(const std::string& text)
  {
    super::setText(text);
    this->title.setText(text);
    this->updateHeaderFlowVisibility();
  }

  void Frame::setText(std::string&& text)
  {
    super::setText(text);
    this->title.setText(std::move(text));
    this->updateHeaderFlowVisibility();
  }

  Frame& Frame::setDragTarget(Window* dragTarget)
  {
    this->dragTarget = dragTarget;
    if (this->style.getDragByTitle())
      this->title.setDragTarget(dragTarget);
    else
      this->title.setDragTarget(nullptr);
    this->filler.setDragTarget(dragTarget);
    this->headerFlow.setDragTarget(dragTarget);
    return *this;
  }

  Window* Frame::getDragTarget()
  {
    if (this->dragTarget == nullptr)
      return nullptr;
    Window* window = dynamic_cast<Window*>(this);
    if (this->dragTarget == window)
      return window;
    return this->dragTarget->getDragTarget();
  }

  const Window* Frame::getDragTarget() const
  {
    if (this->dragTarget == nullptr)
      return nullptr;
    const Window* window = dynamic_cast<const Window*>(this);
    if (this->dragTarget == window)
      return window;
    return this->dragTarget->getDragTarget();
  }

  void Frame::setSpacing(int spacing)
  {
    if (this->direction == GuiDirection::Horizontal)
      static_cast<agui::HorizontalFlow*>(this->layout)->style.setHorizontalSpacing(spacing);
    else
      static_cast<agui::VerticalFlow*>(this->layout)->style.setVerticalSpacing(spacing);
  }

  agui::VerticalFlow* Frame::getVerticalFlow()
  {
    return this->direction == GuiDirection::Vertical ? static_cast<VerticalFlow*>(this->layout) : nullptr;
  }

  agui::HorizontalFlow* Frame::getHorizontalFlow()
  {
    return this->direction == GuiDirection::Horizontal ? static_cast<HorizontalFlow*>(this->layout) : nullptr;
  }
}

#include "Agui/ElementImageSet.hpp"
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/TopContainer.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include "Agui/Widget/Window.hpp"
#include <algorithm>
#include <cassert>
#include <Agui/EventDispatchHelper.hpp>

namespace agui
{
  Window::Window(GuiDirection direction, const agui::FrameStyle* parentStyle, HeightRule heightRule, const std::string& title)
    : Frame(direction, parentStyle, title)
    , heightRule(heightRule)
  {
    this->layout->autoDestructWhenRemovedFromParent();
    this->addPrivateChild(this->layout);
  }

  Window::Window(GuiDirection direction, HeightRule heightRule)
    : Window(direction, nullptr, heightRule)
  {}

  Window::Window(GuiDirection direction, const std::string& title)
    : Frame(direction, title)
    , heightRule(HeightRule::MaxScreenHeightWithExtraSpace)
  {
    this->layout->autoDestructWhenRemovedFromParent();
    this->addPrivateChild(this->layout);
  }

  bool Window::isMovable() const
  {
    return this->getDragTarget() != nullptr;
  }

  bool Window::mouseDown(const MouseEvent& mouseEvent)
  {
    if (!this->isMovable())
      return false;
    this->moving = true;
    this->dragPoint = mouseEvent.getPosition();
    return true;
  }

  bool Window::mouseDrag(const MouseEvent& mouseEvent)
  {
    if (!this->moving || mouseEvent.getSourceWidget() == this)
      return super::mouseDrag(mouseEvent);

    Window* dragTarget = this->getDragTarget();
    int deltaX = mouseEvent.getPosition().x - this->dragPoint.x + dragTarget->getLocation().x;
    int deltaY = mouseEvent.getPosition().y - this->dragPoint.y + dragTarget->getLocation().y;

    const Point before = this->getLocation();
    dragTarget->setLocation(deltaX, deltaY);
    this->anchorToWidgetOrder.clear();
    if (this->getLocation() != before)
      dragTarget->dispatchMouseDrag(mouseEvent.copyWithNewSource(dragTarget));
    return true;
  }

  bool Window::mouseUp(const MouseEvent&)
  {
    if (!this->moving)
      return false;
    this->moving = false;
    return false;
  }

  void Window::setContentSize(int width, int height)
  {
    int maxTitleWidth = this->style.getTitleStyle()->getFont()->getTextWidth(this->title.getText(), RichTextSetting::Enabled) + this->style.getTitleStyle()->getLeftPadding() + this->style.getTitleStyle()->getRightPadding();
    super::setContentSize(std::max(width, maxTitleWidth), height + this->getTopPartHeight());
  }

  void Window::resizeToContents()
  {
    agui::Dimension originalSize = this->getSize();
    super::resizeToContents();
    if (this->isMain && this->autoEnsureWholeWindowIsVisible && (originalSize.width < this->getWidth() || originalSize.height < this->getHeight()))
      this->ensureWholeWindowIsOnScreen();
    if (this->autoCenterOrdered)
    {
      this->autoCenterOrdered = false;
      this->center();
    }
    this->anchor();
  }

  void Window::center()
  {
    if (this->getParent() == Gui::instance->getTop())
    {
      agui::Rectangle rectangleForCentering;
      if (!this->fullScreen)
        rectangleForCentering = this->getGui()->getGraphicsContext()->rectangleForCentering(*this);
      else
        rectangleForCentering = agui::Rectangle(agui::Point(0, 0),
                                                this->getGui()->getGraphicsContext()->getDisplaySize());
      const agui::Point previous = this->getLocation();
      this->setLocation(rectangleForCentering.x + (rectangleForCentering.width - this->getWidth()) / 2,
                        rectangleForCentering.y + (rectangleForCentering.height - this->getHeight()) / 2);
      this->ensureWholeWindowIsOnScreen();
      if (this->getLocation() != previous)
        this->dispatchCentered();
    }
  }

  void Window::anchorToPointInternal()
  {
    if (this->anchorToPointOrder.empty())
      return;
    if (this->anchorType == AnchorType::CenterToCenter)
      this->setLocation(anchorToPointOrder.x - this->getWidth() / 2, this->anchorToPointOrder.y - this->getHeight() / 2);
    else
      assert(false); // type not implemented
    this->ensureWholeWindowIsOnScreen();
    this->anchorToPointOrder = agui::Point::emptyPoint();
  }

  void Window::anchorToWidgetInternal()
  {
    if (this->anchorToWidgetOrder)
      this->positionToWidget(*this->anchorToWidgetOrder, this->anchorType);
  }

  void Window::initAsMain(bool autocenter)
  {
    this->isMain = true;
    Gui::instance->focusManager.onMainWindowCreated(this);
    if (autocenter)
    {
      this->autoCenterEnabled = true;
      this->autoCenterOrdered = true;
    }
    assert(!this->getParent());
    Gui::instance->add(this);
    Gui::instance->requestBringWidgetToFront(this);
    this->setDragTarget(this);
    this->updateMaximumSize();
  }

  void Window::dispatchCentered()
  {
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnCentered))
      listener.onCenter();
  }

  void Window::displaySizeChanged()
  {
    super::displaySizeChanged();
    this->updateMaximumSize();
    if (this->autoCenterEnabled)
      this->autoCenterOrdered = true;
  }

  void Window::ensureWholeWindowIsOnScreen()
  {
    if (this->getGui() == nullptr)
      return;
    if (this->getParent() != this->getGui()->getTop())
      return;
    agui::Point adjustedLocation = this->getLocation();
    adjustedLocation.x = std::max(0, adjustedLocation.x);
    if (this->getWidth() <= Gui::instance->getGraphicsContext()->getDisplaySize().width)
      adjustedLocation.x = std::min(Gui::instance->getGraphicsContext()->getDisplaySize().width - this->getWidth(), adjustedLocation.x);
    else
      adjustedLocation.x = 0;
    adjustedLocation.y = std::max(0, adjustedLocation.y);
    if (this->getHeight() <= Gui::instance->getGraphicsContext()->getDisplaySize().height)
      adjustedLocation.y = std::min(Gui::instance->getGraphicsContext()->getDisplaySize().height - this->getHeight(), adjustedLocation.y);
    else
      adjustedLocation.y = 0;
    if (adjustedLocation != this->getLocation())
      this->setLocation(adjustedLocation.x, adjustedLocation.y);
  }

  Window* Window::getParentWindow()
  {
    Widget* gui = this;
    while (gui = gui->getParent())
      if (Window* window = dynamic_cast<Window*>(gui))
        return window;
    return nullptr;
  }

  uint32_t Window::getMinimumDifferenceBetweenDisplayHeightAndWindowContentsHeight()
  {
    if (Gui::instance->isScreenHeightSmall())
      return 24;
    else
      return 150;
  }

  void Window::updateMaximumSize()
  {
    if (this->fullScreen)
    {
      if (Window* parentWindow = this->getParentWindow())
        if (parentWindow->fullScreen)
        {
          this->style.clearSizes();
          return;
        }
      this->style.setSize(Gui::instance->getGraphicsContext()->getDisplaySize().width,
                          Gui::instance->getGraphicsContext()->getDisplaySize().height);
      return;
    }

    if (this->isMain)
    {
      switch (this->heightRule)
      {
      case HeightRule::MaxScreenHeight:
        this->style.setMaximalHeight(Gui::instance->getGraphicsContext()->getDisplaySize().height); break;
      case HeightRule::MaxScreenHeightWithExtraSpace:
        this->style.setMaximalHeight(Gui::instance->getGraphicsContext()->getDisplaySize().height -
                                     Gui::scale * getMinimumDifferenceBetweenDisplayHeightAndWindowContentsHeight()); break;
      case HeightRule::ReasonableMinimumAndMaxScreenWithExtraSpace:
        this->style.setMinimalHeight(Gui::scale* 500);
        this->style.setMaximalHeight(Gui::instance->getGraphicsContext()->getDisplaySize().height -
                                     Gui::scale * getMinimumDifferenceBetweenDisplayHeightAndWindowContentsHeight());
        break;
      case HeightRule::ConstantScreenHeightWithExtraSpace:
        this->style.setConstantHeight(Gui::instance->getGraphicsContext()->getDisplaySize().height -
                                      Gui::scale * getMinimumDifferenceBetweenDisplayHeightAndWindowContentsHeight()); break;
      case HeightRule::ConstantScreenHeight:
        this->style.setConstantHeight(Gui::instance->getGraphicsContext()->getDisplaySize().height); break;
      }
    }
  }

  void Window::clear()
  {
    this->layout->clear();
  }

  void Window::anchorToPoint(agui::Point point, AnchorType anchorType)
  {
    this->anchorToPointOrder = point;
    this->anchorType = anchorType;
  }

  void Window::anchorToPoint(const agui::Widget* widget, AnchorType anchorType)
  {
    if (anchorType == AnchorType::CenterToCenter)
      this->anchorToPoint(widget->getAbsoluteRectangle().getCenter(), anchorType);
    else
      assert(false); // type not implemented
  }

  void Window::anchorToPoint(const agui::Widget& widget, AnchorType anchorType)
  {
    if (anchorType == AnchorType::CenterToCenter)
      this->anchorToPoint(widget.getAbsoluteRectangle().getCenter(), anchorType);
    else
      assert(false); // type not implemented
  }

  void Window::anchorToWidget(agui::Widget& widget, AnchorType anchorType)
  {
    this->anchorToWidgetOrder = &widget;
    this->anchorType = anchorType;
    if (Gui* gui = this->getGui())
      gui->addAnchoredWidget(*this);
  }

  void Window::positionToWidget(const agui::Widget* widget, AnchorType anchorType)
  {
    if (!this->getParent())
    {
      assert(false); // positioning widget that is not in the gui
      return;
    }

    Point absoluteTarget;
    Rectangle widgetRectangle(widget->getAbsoluteRectangle());

    switch (anchorType)
    {
      case AnchorType::CenterToCenter:
        absoluteTarget.x = widgetRectangle.getCenterX() - this->getWidth() / 2;
        absoluteTarget.y = widgetRectangle.getCenterY() - this->getHeight() / 2;
        break;
      case AnchorType::BottomLeftToRight:
        absoluteTarget.x = widgetRectangle.x;
        if (widgetRectangle.getBottom() + this->getHeight() > Gui::instance->getGraphicsContext()->getDisplaySize().height)
          absoluteTarget.y = widgetRectangle.y - this->getHeight();
        else
          absoluteTarget.y = widgetRectangle.getBottom();
        break;
      case AnchorType::BottomRightToLeft:
        absoluteTarget.x = widgetRectangle.getRight() - this->getWidth();
        if (widgetRectangle.getBottom() + this->getHeight() > Gui::instance->getGraphicsContext()->getDisplaySize().height)
          absoluteTarget.y = widgetRectangle.y - this->getHeight();
        else
          absoluteTarget.y = widgetRectangle.getBottom();
        break;
      case AnchorType::CenterLeftToLeft:
        absoluteTarget.x = widgetRectangle.x - (this->getWidth() + this->style.getRightMargin());
        absoluteTarget.y = widgetRectangle.getCenterY() - this->getHeight() / 2;
        break;
      case AnchorType::TopLeftToRight:
        absoluteTarget.x = widgetRectangle.x;
        if (widgetRectangle.getTop() - this->getHeight() < 0)
          absoluteTarget.y = widgetRectangle.getBottom();
        else
          absoluteTarget.y = widgetRectangle.y - this->getHeight();
        break;
      case AnchorType::TopLeftToLeft:
        absoluteTarget.x = widgetRectangle.x - (this->getWidth() + this->style.getRightMargin());
        absoluteTarget.y = widgetRectangle.y;
        break;
      case AnchorType::TopRightToRight:
        absoluteTarget.x = widgetRectangle.x + widgetRectangle.width + this->style.getLeftMargin();
        absoluteTarget.y = widgetRectangle.y;
        break;
      default:
        assert(false); // type not implemented
        break;
    }
    Point relativeTarget = absoluteTarget - this->getParent()->getAbsolutePosition() - this->getParent()->getContentRectangle().getLeftTop();
    this->setLocation(relativeTarget);
    this->ensureWholeWindowIsOnScreen();
  }

  void Window::anchor()
  {
    assert(this->anchorToPointOrder.empty() || !this->anchorToWidgetOrder); // setting both makes no sense
    this->anchorToPointInternal();
    this->anchorToWidgetInternal();
  }

  bool Window::checkAnchor()
  {
    if (!this->getParent())
      return false;
    if (!this->anchorToWidgetOrder)
      return false;
    this->anchor();
    return true;
  }

  void Window::checkFullscreenInParent()
  {
    if (!this->fullScreen)
      return;
    Window* parentWindow = this->getParentWindow();
    if (!parentWindow)
      return;
    parentWindow->setFullScreen(true);
    this->updateMaximumSize();
  }

  void Window::setFullScreen(bool value)
  {
    this->fullScreen = value;
    this->updateMaximumSize();
  }
}

#include <Agui/Effect.hpp>
#include <Agui/EventDispatchHelper.hpp>
#include <Agui/Exception.hpp>
#include <Agui/FocusManager.hpp>
#include <Agui/Graphics.hpp>
#include <Agui/Gui.hpp>
#include <Agui/Image.hpp>
#include <Agui/TextInputInfo.hpp>
#include <Agui/Layout.hpp>
#include <Agui/PaintEvent.hpp>
#include <Agui/Style.hpp>
#include <Agui/ToolTipCreatorBase.hpp>
#include <Agui/TopContainer.hpp>
#include <Agui/UTF8.hpp>
#include <Agui/Widget.hpp>
#include <Agui/Widget/CheckBox.hpp>
#include <Agui/Widget/EmptyWidget.hpp>
#include <Agui/Widget/HorizontalFlow.hpp>
#include <Agui/Widget/ImageWidget.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/Line.hpp>
#include <Agui/Widget/Tab.hpp>
#include <Agui/Widget/Table.hpp>
#include <Agui/Widget/TextBox.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <Agui/Widget/VerticalFlow.hpp>
#include <Agui/Widget/VerticalScrollPane.hpp>
#include <Agui/Util.hpp>
#include <Agui/StringUtil.hpp>
#include <Agui/StringMatcher.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <queue>
#include <Agui/DrawCulledChildren.hpp>
#include <Agui/Math.hpp>
#include <Agui/ScopedSetter.hpp>

namespace agui
{
#ifdef DEBUG
  std::unordered_set<Widget*> Widget::widgets;
#endif

  bool Widget::setShownBySearch(bool value)
  {
    if (this->isHiddenBySearch() != value)
      return value;
    if (!this->canBeHiddenBySearch())
      return false;
    if (value)
      this->showBySearch();
    else
      this->hideBySearch();
    return value;
  }

  void Widget::hideBySearch()
  {
    if (this->isHiddenBySearch())
      return;
    this->usageBitMask |= HIDDEN_BY_SEARCH;
    this->triggerResize();
  }

  void Widget::showBySearch()
  {
    if (!this->isHiddenBySearch())
      return;
    this->usageBitMask &= ~HIDDEN_BY_SEARCH;
    this->triggerResize();
  }

  bool Widget::contains(const LowercaseString& filter)
  {
    if (!this->getText().empty())
      if (StringMatcher::matchesSearchPattern(this->getText(), filter) != StringMatcherResult::NoMatch)
        return true;
    for (std::vector<Widget*>* kids : {&this->getPrivateChildren(), &this->getChildren()})
      for (Widget* widget : *kids)
        if (!widget->isIgnoredBySearch())
          if (widget->contains(filter))
            return true;
    return false;
  }

  bool Widget::genericSearch(const LowercaseString& filter)
  {
    std::optional<bool> result;
    bool containsHideBlocker = false;

    if (this->isAtomicSearch())
      result = this->contains(filter);
    else
    {
      for (std::vector<Widget*>* kids : {&this->getPrivateChildren(), &this->getChildren()})
        for (Widget* widget : *kids)
          if (!widget->isIgnoredBySearch())
          {
            if (widget->genericSearch(filter))
              result = true;
            else if (!result)
              result = false;
          }
          else if (!widget->canBeHiddenBySearch())
            containsHideBlocker = true;
    }

    if (!containsHideBlocker && result)
      this->setShownBySearch(*result);
    return !this->isHiddenBySearch();
  }

  agui::Widget& Widget::neverHideBySearch()
  {
    this->getStyle()->setNeverHiddenBySearch(true);
    return *this;
  }

  agui::Widget& Widget::ignoreBySearch()
  {
    this->getStyle()->setIgnoredBySearch(true);
    return *this;
  }

  agui::Widget& Widget::dontIgnoreBySearch()
  {
    this->getStyle()->setIgnoredBySearch(false);
    return *this;
  }

  bool Widget::isIgnoredBySearch() const
  {
    if (this->getStyle()->isIgnoredBySearch())
      return true;
    for (Widget* widget : this->getPrivateChildren())
      if (!widget->isIgnoredBySearch())
        return false;

    for (Widget* widget : *this)
      if (!widget->isIgnoredBySearch())
        return false;
    return this->getChildCount() != 0 || this->getPrivateChildCount() != 0;
  }

  Widget* Widget::getGameControllerHoveredChild()
  {
    switch (this->gameControllerInteraction)
    {
      case GameControllerInteraction::Always:
        return this;
      case GameControllerInteraction::Normal:
        return this->getGameControllerHoveredChildInternal();
      case GameControllerInteraction::Never:
        return nullptr;
    }
    assert(false);
    return nullptr;
  }

  bool Widget::isInset() const
  {
    if (const ElementImageSet* elementImageSet = this->getBorderImageSet())
      return elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Outer;
    return false;
  }

  const Style& Widget::defaultWidgetStyle = Widget::getStyleStatic();
  const Style& Widget::getStyleStatic()
  {
    static Style result(nullptr, nullptr);
    result.setHorizontalAlign(HorizontalAlign::Left);
    result.setVerticalAlign(VerticalAlign::Top);
    result.setHorizontallyStretchable(StretchRule::Auto);
    result.setVerticallyStretchable(StretchRule::Auto);
    result.setHorizontallySquashable(StretchRule::Auto);
    result.setVerticallySquashable(StretchRule::Auto);
    result.setMinimalWidth(0);
    result.setMinimalHeight(0);
    result.setMaximalWidth(0);
    result.setMaximalHeight(0);
    result.setNaturalWidth(0);
    result.setNaturalHeight(0);
    result.setTopPadding(0);
    result.setRightPadding(0);
    result.setBottomPadding(0);
    result.setLeftPadding(0);
    result.setTopMargin(0);
    result.setRightMargin(0);
    result.setBottomMargin(0);
    result.setLeftMargin(0);
    return result;
  }

  std::string Widget::getParentPathString() const
  {
    std::vector<const Widget*> widgets;
    for (const agui::Widget* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
      widgets.push_back(parent);

    auto removeClassPrefix = [](std::string name) -> std::string
    {
      if (StringUtil::startsWith(name, "class "))
        name.erase(name.begin(), name.begin() + strlen("class "));
      return name;
    };

    std::string result = "ROOT";
    auto it = widgets.rbegin();
    auto end = widgets.rend();
    for (; it != end; ++it)
      result += " -> " + removeClassPrefix(Util::demangle(typeid(**it).name()));

    result += " -> " + removeClassPrefix(Util::demangle(typeid(*this).name()));

    return result;
  }

  void Widget::setGuiInstanceRecursively(agui::Gui* instance)
  {
    // Note: no shortcuts, no short-circuiting! This method needs to modify all child widgets, no exceptions!
    this->guiInstance = instance;
    for (Widget* widget : this->getPrivateChildren())
      widget->setGuiInstanceRecursively(instance);
    for (Widget* widget : *this)
      widget->setGuiInstanceRecursively(instance);
  }

  void Widget::handleRemoval()
  {
    bool removedToolTip = this->removeToolTipWidget();

    delete this->toolTipCreator;
    this->toolTipCreator = nullptr;

    if (this->getGui() && *this->getGui()->widgetUnderMouse == this)
    {
      this->getGui()->clearWidgetUnderMouse();
      if (removedToolTip)
        this->getGui()->flagForForceHover();
    }
  }

  Widget::~Widget(void)
  {
    this->handleRemoval();

    // I need to avoid sending any focus related events now as I'm already destructing the widget
    // and unexpected things might happen if this widget is removed as part of the object owning
    // both this widget and the listener.
    this->actionListeners.clear();

    if (getParent())
    {
      if (getParent()->containsPrivateChild(this))
        getParent()->removePrivateChild(this);
      else
        getParent()->remove(this);
    }

    for (Widget* widget : this->getPrivateChildren())
      widget->clearParentWidget();

    for (Widget* widget : *this)
      widget->clearParentWidget();

#ifdef DEBUG
    Widget::widgets.erase(this);
#endif
  }

  void Widget::recalculateClippingRect() const
  {
    if (this->clippingRectangle)
      return;
    this->clippingRectangle = this->getSizeRectangle();
    for (auto* container : {&this->getPrivateChildren(), &this->getChildren()})
      for (Widget* widget : *container)
        if (widget->isVisible())
        {
          widget->recalculateClippingRect();
          agui::Rectangle widgetRect = *widget->clippingRectangle;
          widgetRect += widget->getLocation();
          *this->clippingRectangle |= widgetRect;
        }
  }

  Rectangle Widget::getClippingRect() const
  {
    this->recalculateClippingRect();
    return *this->clippingRectangle;
  }

  void Widget::paint(const PaintEvent& paintEvent, const Point& absolutePosition)
  {
    this->paintBackground(paintEvent, absolutePosition);
    int leftPadding, topPadding;
    this->getPaddings(&leftPadding, nullptr, &topPadding, nullptr);
    paintEvent.graphics()->setOffset(Point(absolutePosition.x + leftPadding, absolutePosition.y + topPadding));
    this->paintComponent(paintEvent, absolutePosition);
  }

  void Widget::setText(const std::string& text)
  {
    if (text == this->text)
      return;
    this->text = text;
    this->textLen = int(UTF8::length(this->getText()));
    this->triggerResize();
  }

  void Widget::setText(std::string&& text)
  {
    if (text == this->text)
      return;
    this->text = std::move(text);
    this->textLen = int(UTF8::length(this->getText()));
    this->triggerResize();
  }

  const std::string& Widget::getText() const
  {
    return this->text;
  }

  void Widget::moveWidgetBefore(Widget* widget, Widget* moveBeforeWidget)
  {
    if (widget && this->containsChildWidget(widget))
    {
      assert(!this->childrenBeingIterated);
      WidgetArray::iterator removeIt = this->children.begin();
      std::advance(removeIt, this->getChildWidgetIndex(widget));
      this->children.erase(removeIt);

      WidgetArray::iterator insertIt = this->children.begin();
      std::advance(insertIt,
                   moveBeforeWidget && this->containsChildWidget(moveBeforeWidget)
                   ? this->getChildWidgetIndex(moveBeforeWidget)
                   : this->children.size());
      this->children.insert(insertIt, widget);
      this->triggerResize();

      if (getGui())
        getGui()->widgetLocationChanged();
    }
  }

  void Widget::swapChildren(size_t index1, size_t index2)
  {
    if (index1 >= this->children.size() || index2 >= this->children.size() || index1 == index2)
      return;
    std::swap(this->children[index1], this->children[index2]);
    this->triggerResize();
  }

  void Widget::add(Widget* widget)
  {
    if (widget && widget->parentWidget == nullptr &&
        ((this->usageBitMask & SKIP_DUPLICATE_WIDGET_CHECK) != 0 || !this->containsChildWidget(widget)))
      this->addUnchecked(widget); // direct call - don't do virtual dispatch
  }

  void Widget::addUnchecked(Widget* widget)
  {
    assert(!this->childrenBeingIterated);
    this->children.push_back(widget);
    widget->parentWidget = this;
    assert(widget->guiInstance == nullptr);
    if (this->isParentHovered())
      widget->setParentHovered(true);
    if (this->getGui())
      this->getGui()->widgetLocationChanged();
    this->triggerResize();
  }

  void Widget::insert(Widget* widget, uint32_t index)
  {
    if (widget->parentWidget == nullptr && !this->containsChildWidget(widget))
    {
      assert(!this->childrenBeingIterated);
      assert(this->getChildCount() >= index);
      this->children.insert(this->children.begin() + index, widget);
      widget->parentWidget = this;
      assert(widget->guiInstance == nullptr);
      if (this->getGui())
        this->getGui()->widgetLocationChanged();
      this->triggerResize();
    }
  }

  void Widget::addFront(Widget* widget)
  {
    if (widget->parentWidget == nullptr && !this->containsChildWidget(widget))
    {
      assert(!this->childrenBeingIterated);
      this->children.insert(this->children.begin(), widget);
      widget->parentWidget = this;
      assert(widget->guiInstance == nullptr);
      if (this->getGui())
        this->getGui()->widgetLocationChanged();
      this->triggerResize();
    }
  }

  Widget* Widget::getParent() const
  {
    return parentWidget;
  }

  bool Widget::removeChildInternal(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    const size_t size = this->children.size();
    for (size_t i = 0; i < size; ++i)
      if (this->children[i] == widget)
      {
        assert(!this->childrenBeingIterated);
        this->children.erase(this->children.begin() + i);

        this->handleChildRemoved(widget, keepWidgetAlive);
        this->triggerResize();

        return true;
      }

    return false;
  }

  bool Widget::removePrivateChildInternal(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    const size_t size = this->privateChildren.size();
    for (size_t i = 0; i < size; ++i)
      if (this->privateChildren[i] == widget)
      {
        assert(!this->privateChildrenBeingIterated);
        this->privateChildren.erase(this->privateChildren.begin() + i);

        widget->clearParentWidget(keepWidgetAlive);
        this->triggerResize();

        return true;
      }

    return false;
  }

  void Widget::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    if (this->removeChildInternal(widget, keepWidgetAlive) ||
        this->removePrivateChildInternal(widget, keepWidgetAlive))
      if (this->getGui())
        this->getGui()->widgetLocationChanged();
  }

  void Widget::removeFromParent(KeepWidgetAlive keepWidgetAlive)
  {
    if (this->parentWidget)
      this->parentWidget->remove(this, keepWidgetAlive);
  }

  void Widget::checkLostFocusRecursive()
  {
    if (FocusManager* focusManager = this->getFocusManager())
    {
      Widget* focused = focusManager->getFocusedWidget();
      if (!focused)
        return;
      if (focused == this)
      {
        focusManager->clearFocusedWidget();
        return;
      }
    }

    for (Widget* widget : this->getPrivateChildren())
      widget->checkLostFocusRecursive();
    for (Widget* widget : *this)
      widget->checkLostFocusRecursive();
  }

  void Widget::checkLostWidgetUnderMouseRecursive()
  {
    Widget* top = this->getTopWidget();
    if (top && top->guiInstance)
    {
      if (!top->guiInstance->widgetUnderMouse ||
          *top->guiInstance->widgetUnderMouse == top)
        return;
      if (*top->guiInstance->widgetUnderMouse == this)
      {
        bool removedToolTip = this->removeToolTipWidget();
        if (removedToolTip)
          top->getGui()->flagForForceHover();
        top->guiInstance->clearWidgetUnderMouse();
        MouseEvent mouseEvent;
        this->mouseLeave(mouseEvent);
        return;
      }
    }

    for (Widget* widget : this->getPrivateChildren())
      widget->checkLostWidgetUnderMouseRecursive();
    for (Widget* widget : *this)
      widget->checkLostWidgetUnderMouseRecursive();
  }

  void Widget::clearParentWidget(KeepWidgetAlive keepWidgetAlive)
  {
    this->checkLostFocusRecursive();
    this->checkLostWidgetUnderMouseRecursive();
    if (!keepWidgetAlive)
      this->destroyIfautoDestructWhenRemovedFromParent();

    this->parentWidget = nullptr;
    this->setGuiInstanceRecursively(nullptr);
  }

  void Widget::destroyIfautoDestructWhenRemovedFromParent()
  {
    if ((this->usageBitMask & AUTO_DESTRUCT_WHEN_REMOVED_FROM_PARENT) != 0)
      this->flagForDestruction();
  }

  bool Widget::containsChildWidget(Widget* widget) const
  {
    for (Widget* child : this->children)
      if (child == widget)
        return true;

    return false;
  }

  int Widget::getChildWidgetIndex(const Widget* widget) const
  {
    // returns index or -1 if not found
    int count = 0;
    for (Widget* child : this->children)
    {
      if (child == widget)
        return count;
      count++;
    }
    return -1;
  }

  const agui::Widget* Widget::getNextSibling() const
  {
    agui::Widget* parent = this->getParent();
    if (!parent)
      return nullptr;
    uint32_t index = parent->getChildWidgetIndex(this);
    if (parent->getChildCount() <= index + 1)
      return nullptr;
    return parent->getChildAt(index + 1);
  }

  TextInputInfo Widget::queryTextInputInfo()
  {
    return TextInputInfo();
  }

  int Widget::getIndexInParent() const
  {
    if (!getParent())
      return -1;

    return getParent()->getChildWidgetIndex(this);
  }

  bool Widget::mouseLeave(const MouseEvent&)
  {
    Gui* gui = this->getGui();
    if (!gui || gui->getLockWidget() != this || !gui->mouseInWindow)
      this->removeToolTipWidget();
    return true;
  }

  void Widget::focusGained(TabbedIn)
  {
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnFocusGain))
      listener.onFocusGain();
  }

  void Widget::focusLost()
  {
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnFocusLose))
      listener.onFocusLose();
  }

  Widget* Widget::getTopWidget() const
  {
    Widget* parent = this->getParent();
    if (parent)
      while (parent->getParent() != nullptr)
        parent = parent->getParent();
    return parent;
  }

  bool Widget::intersectionWithPoint(const Point& point) const
  {
    Rectangle clippingRectangle = this->getClippingRect() + this->location;
    // Rectangle::collide() is not quite the same, although maybe we should change this (or that) to be equivalent?
    return point.x >= clippingRectangle.x &&
           point.x < clippingRectangle.getRight() &&
           point.y >= clippingRectangle.y &&
           point.y < clippingRectangle.getBottom();
  }

  bool Widget::isUnderMouse(const Point& point) const
  {
    return this->isVisible() &&
           this->shouldRender() &&
           !this->isIgnoredByInteraction() &&
           this->intersectionWithPoint(point);
  }

  Rectangle Widget::getAbsoluteRectangle() const
  {
    return this->getRectangleRelativeTo(this->getAbsolutePosition());
  }

  Rectangle Widget::getRectangleRelativeTo(const Point& relativeTo) const
  {
    return Rectangle(relativeTo.x, relativeTo.y, this->getWidth(), this->getHeight());
  }

  void Widget::setSize(int width, int height, SetSizeInfo)
  {
    Dimension originalSize = this->size;
    this->setSizeInternal(width, height);

    if (this->size != originalSize)
      if (this->getGui() && this->getGui()->getLockWidget() == nullptr)
        this->getGui()->widgetLocationChanged();
  }

  void Widget::setSizeInternal(int width, int height)
  {
    Dimension originalSize = this->size;
    if ((this->usageBitMask & DONT_DECREASE_WIDTH) != 0 && width < this->size.width)
      width = this->size.width;
    if ((this->usageBitMask & DONT_DECREASE_HEIGHT) != 0 && height < this->size.height)
      height = this->size.height;

    this->size.set(width, height);
    if (Style* style = this->getStyle())
      this->applySizeRestrictionsInternalWithoutTriggerResize(*style); // don't call the event, as we call it here to merge two two changes
    if (this->size != originalSize)
      this->onSizeChanged(originalSize);
  }

  const Point& Widget::getLocation() const
  {
    return location;
  }

  void Widget::setLocation(const Point& location)
  {
    if (this->location == location)
      return;

    this->location = location;
    if (this->getGui() &&
        (this->getGui()->getLockWidget() == nullptr ||
         this->getGui()->getLockWidget() == this))
      this->getGui()->widgetLocationChanged();
  }

  void Widget::setLocation(int x, int y)
  {
    this->setLocation(Point(x, y));
  }

  bool Widget::keyDown(const KeyEvent& keyEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->keyDown(keyEvent);
    return false;
  }

  bool Widget::keyUp(const KeyEvent& keyEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->keyUp(keyEvent);
    return false;
  }

  bool Widget::keyRepeat(const KeyEvent& keyEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->keyRepeat(keyEvent);
    return false;
  }

  bool Widget::mouseMove(const MouseEvent& mouseEvent)
  {
    this->updateTooltipPosition(mouseEvent.getPosition());
    return false;
  }

  bool Widget::mouseWheelUp(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseWheelUp(mouseEvent);
    return false;
  }

  bool Widget::mouseWheelDown(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseWheelDown(mouseEvent);
    return false;
  }

  bool Widget::mouseWheelLeft(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseWheelLeft(mouseEvent);
    return false;
  }

  bool Widget::mouseWheelRight(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseWheelRight(mouseEvent);
    return false;
  }

  bool Widget::mouseClick(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseClick(mouseEvent);
    return false;
  }

  agui::Widget& Widget::setEnabled(bool enabled)
  {
    if (this->isEnabled() == enabled)
      return *this;

    if (!enabled && this->isFocused())
    {
      if (FocusManager* focusManager = this->getFocusManager())
        focusManager->clearFocusedWidget();
    }
    else if (!enabled)
    {
      std::queue<Widget*> q;
      q.push(this);
      while (!q.empty())
      {
        Widget* c = q.front();
        if (c->isFocused())
          if (FocusManager* focusManager = this->getFocusManager())
          {
            focusManager->clearFocusedWidget();
            break;
          }
        q.pop();
        for (Widget* child : c->children)
          q.push(child);

        for (Widget* child : c->privateChildren)
          q.push(child);
      }

    }

    if (!enabled)
      this->usageBitMask &= ~ENABLED;
    else
      this->usageBitMask |= ENABLED;
    return *this;
  }

  agui::Widget* Widget::getWidgetRecursively(const std::function<bool(agui::Widget*)>& predicate)
  {
    assert(predicate);
    if (predicate(this))
      return this;

    for (auto* container : {&this->getPrivateChildren(), &this->getChildren()})
      for (Widget* widget : *container)
        if (widget->isVisible())
          if (Widget* result = widget->getWidgetRecursively(predicate))
            return result;

    return nullptr;
  }

  void Widget::recursiveSetEnabled(bool enabled)
  {
    this->setEnabled(enabled);
    for (Widget* widget : this->getPrivateChildren())
      widget->recursiveSetEnabled(enabled);
    for (Widget* widget : *this)
      widget->recursiveSetEnabled(enabled);
  }

  bool Widget::isEnabled() const
  {
    return this->usageBitMask & ENABLED;
  }

  bool Widget::isModal() const
  {
    if (FocusManager* focusManager = this->getFocusManager())
      return focusManager->getModalWidget() == this;
    return false;
  }

  Widget* Widget::getChildAt(uint32_t index) const
  {
    if (index >= this->children.size())
      return nullptr;

    return this->children[index];
  }

  uint32_t Widget::getChildCount() const
  {
    return uint32_t(children.size());
  }

  Gui* Widget::getGui() const
  {
    if (this->guiInstance)
      return this->guiInstance;

    Widget* firstParentWithGuiInstance = this->getParent();
    while (firstParentWithGuiInstance != nullptr && firstParentWithGuiInstance->guiInstance == nullptr)
      firstParentWithGuiInstance = firstParentWithGuiInstance->getParent();

    if (firstParentWithGuiInstance)
    {
      this->guiInstance = firstParentWithGuiInstance->guiInstance;
      return this->guiInstance;
    }

    return nullptr;
  }

  bool Widget::mouseDrag(const MouseEvent& mouseEvent)
  {
    this->updateTooltipPosition(mouseEvent.getPosition());
    return false;
  }

  bool Widget::isFocusable() const
  {
    return this->usageBitMask & FOCUSABLE;
  }

  void Widget::setFocusable(bool focusable)
  {
    if (focusable)
      this->usageBitMask |= FOCUSABLE;
    else
      this->usageBitMask &= ~FOCUSABLE;
  }

  void Widget::keepFocusWhenClickingOutsideOnNonFocusableWidget()
  {
    this->usageBitMask |= KEEP_FOCUS_WHEN_CLICKING_OUTSIDE_ON_NON_FOCUSABLE_WIDGET;
  }

  void Widget::dontKeepFocusWhenClickingOutsideOnNonFocusableWidget()
  {
    this->usageBitMask &= ~KEEP_FOCUS_WHEN_CLICKING_OUTSIDE_ON_NON_FOCUSABLE_WIDGET;
  }

  bool Widget::shoulKeepFocusWhenClickingOutsideOnNonFocusableWidget()
  {
    return (this->usageBitMask & KEEP_FOCUS_WHEN_CLICKING_OUTSIDE_ON_NON_FOCUSABLE_WIDGET) != 0;
  }

  void Widget::setFocusParentWhenNotFocusable(bool value)
  {
    if (value)
      this->usageBitMask |= FOCUS_PARENT_WHEN_NOT_FOCUSABLE;
    else
      this->usageBitMask &= ~FOCUS_PARENT_WHEN_NOT_FOCUSABLE;
  }

  bool Widget::getFocusParentWhenNotFocusable() const
  {
    return this->usageBitMask & FOCUS_PARENT_WHEN_NOT_FOCUSABLE;
  }

  void Widget::focus(TabbedIn tabbedIn)
  {
    if (FocusManager* focusManager = this->getFocusManager())
      focusManager->setFocusedWidget(this, tabbedIn);
  }

  void Widget::clearFocus()
  {
    if (FocusManager* focusManager = this->getFocusManager())
      if (focusManager->getFocusedWidget() == this)
        focusManager->setFocusedWidget(nullptr);
  }

  bool Widget::mouseHover(const MouseEvent&)
  {
    this->checkCreateTooltip();
    return false;
  }

  bool Widget::mouseDoubleClick(const MouseEvent& mouseEvent)
  {
    if (this->parentWidget)
      return this->parentWidget->mouseDoubleClick(mouseEvent);
    return false;
  }

  bool Widget::modalMouseDown(const MouseEvent&)
  {
    return false;
  }

  void Widget::requestModalFocus(ModalFocusPriority priority, bool isDropDownListBox)
  {
    if (FocusManager* focusManager = this->getFocusManager())
    {
      focusManager->requestModalFocus(this, priority, isDropDownListBox);
      if (Gui* gui = this->getGui())
        gui->modalChanged(); // update widget under mouse
    }
  }

  void Widget::releaseModalFocus()
  {
    if (FocusManager* focusManager = this->getFocusManager())
      focusManager->releaseModalFocus(this);
  }

  void Widget::requestTopModalFocusOrNone()
  {
    if (FocusManager* focusManager = this->getFocusManager())
      if (focusManager->requestTopModalFocusOrNone(this))
        if (Gui* gui = this->getGui())
          gui->modalChanged(); // update widget under mouse
  }

  MouseEvent Widget::addSourceToMouseEvent(const MouseEvent& mouseEvent)
  {
    return MouseEvent(mouseEvent.getPosition(),
                      mouseEvent.getMouseWheelChange(),
                      mouseEvent.getButton(),
                      mouseEvent.getEvent(),
                      mouseEvent.getTimeStamp(),
                      mouseEvent.alt(),
                      mouseEvent.control(),
                      mouseEvent.shift(),
                      this);
  }

  KeyEvent Widget::addSourceToKeyEvent(const KeyEvent& keyEvent)
  {
    return KeyEvent(keyEvent.getKey(), keyEvent.getExtendedKey(),
                    keyEvent.getBackendKeycode(),
                    keyEvent.getBackendScancode(),
                    keyEvent.getUnichar(), keyEvent.getTimeStamp(),
                    keyEvent.alt(), keyEvent.control(), keyEvent.shift(), keyEvent.meta(),
                    this, keyEvent.isConsumed());
  }

  void Widget::focusNext()
  {
    if (this->children.empty())
      return;

    WidgetArray::iterator begin = this->children.begin();
    WidgetArray::iterator end = this->children.end();

    WidgetArray::iterator startWidget = begin;

    for (WidgetArray::iterator it = begin; it != end; ++it)
      if ((*it)->isFocused())
      {
        if (this->children.size() == 1)
          return;
        startWidget = it;
        break;
      }

    WidgetArray::iterator currentWidget = startWidget;
    const Widget* focusedWidget = getFocusedWidget();

    do
    {
      if (currentWidget == end)
        currentWidget = begin;

      if ((*currentWidget)->isFocusable() &&
          !(*currentWidget)->shouldSkipForFocusNext() &&
          (*currentWidget) != focusedWidget)
      {
        (*currentWidget)->focus();
        return;
      }

      if (currentWidget != end)
        ++currentWidget;
    }
    while (currentWidget != startWidget);
  }

  void Widget::focusPrevious()
  {
    if (this->children.empty())
      return;

    WidgetArray::reverse_iterator begin = this->children.rbegin();
    WidgetArray::reverse_iterator end = this->children.rend();

    WidgetArray::reverse_iterator startWidget = begin;

    for (WidgetArray::reverse_iterator it = begin; it != end; ++it)
      if ((*it)->isFocused())
      {
        if (this->children.size() == 1)
          return;
        startWidget = it;
        break;
      }

    WidgetArray::reverse_iterator currentWidget = startWidget;
    const Widget* focusedWidget = this->getFocusedWidget();

    do
    {
      if (currentWidget == end)
        currentWidget = begin;

      if ((*currentWidget)->isFocusable() &&
          !(*currentWidget)->shouldSkipForFocusNext() &&
          (*currentWidget) != focusedWidget)
      {
        (*currentWidget)->focus();
        return;
      }

      if (currentWidget != end)
        ++currentWidget;
    }
    while (currentWidget != startWidget);
  }

  Widget* Widget::getFocusedWidget() const
  {
    if (FocusManager* focusManager = this->getFocusManager())
      return focusManager->getFocusedWidget();
    return nullptr;
  }

  bool Widget::isFocused() const
  {
    if (FocusManager* focusManager = this->getFocusManager())
      return focusManager->getFocusedWidget() == this;
    return false;
  }

  bool Widget::_dispatchKeyboardListenerEvent(KeyEvent::KeyboardEventEnum event, const KeyEvent& keyEvent)
  {
    if (this->actionListeners.empty())
      return false;

    KeyEvent kArgs = addSourceToKeyEvent(keyEvent);

      switch (event)
      {
        case KeyEvent::KEY_DOWN:
          for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnKeyDown))
            listener.onKeyDown(kArgs);
           break;
        case KeyEvent::KEY_UP:
          for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnKeyUp))
            listener.onKeyUp(kArgs);
          break;
        case KeyEvent::KEY_REPEAT:
          for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnKeyRepeat))
            listener.onKeyRepeat(kArgs);
          break;
        default: break;
      }
    return false;
  }

  void Widget::setBackWidget(Widget* widget)
  {
    auto moveToBack = [](WidgetArray& widgets, Widget* widget)
    {
      const size_t size = widgets.size();
      for (size_t i = 0; i < size; ++i)
        if (widgets[i] == widget)
        {
          if (i != 0)
          {
            widgets.erase(widgets.begin() + i);
            widgets.insert(widgets.begin(), widget);
          }
          return true;
        }

      return false;
    };

    assert(!this->childrenBeingIterated);
    assert(!this->privateChildrenBeingIterated);

    if (moveToBack(this->children, widget))
      return;

    if (moveToBack(this->privateChildren, widget))
      return;
  }

  void Widget::setFrontWidget(Widget* widget)
  {
    // This will only work for widgets in 'top' to prevent reorganizing the internals of flows/tables etc.
    if (this->getGui() && this->getGui()->getTop() != this)
      return;

    auto moveToFront = [](WidgetArray& widgets, Widget* widget)
    {
      if (!widgets.empty() && widgets.back() == widget)
        return true;

      const size_t size = widgets.size() - 1u;
      for (size_t i = 0; i < size; ++i)
        if (widgets[i] == widget)
        {
          widgets.erase(widgets.begin() + i);
          widgets.push_back(widget);
          return true;
        }

      return false;
    };

    assert(!this->childrenBeingIterated);
    assert(!this->privateChildrenBeingIterated);

    if (moveToFront(this->children, widget))
      return;

    if (moveToFront(this->privateChildren, widget))
      return;
  }

  void Widget::bringToFront()
  {
    if (this->isOnTop())
      return;
    if (this->getParent())
      this->getParent()->setFrontWidget(this);
    if (!this->isToolTip())
      if (FocusManager* focusManager = this->getFocusManager())
        focusManager->checkThatModalFocusedWigetIsOnTop();
    if (Gui* gui = this->getGui()) // tooltip is always on top of everything
                                   // Even if something else is put on top, the tooltip goes above it.
      if (gui->currentTooltip && gui->currentTooltip->doTooltipLogic())
        gui->currentTooltip->bringToFront();
  }

  bool Widget::isOnTop() const
  {
    if (!this->getParent())
      return true;
    if (this->getParent()->getChildren().empty())
      return true;
    return this->getParent()->getChildren().back() == this;
  }

  void Widget::sendToBack()
  {
    if (this->getParent())
      this->getParent()->setBackWidget(this);
  }

  Point Widget::getAbsolutePosition() const
  {
    return this->getRelativePositionTo(nullptr);
  }

  Point Widget::getRelativePositionTo(const Widget* targetAncestor) const
  {
    Point location;
    for (const Widget* widget = this; widget != targetAncestor && widget != nullptr; widget = widget->getParent())
      if (const Widget* widgetParent = widget->getParent())
        location += widgetParent->getChildRelativePosition(widget);
    return location;
  }

  Rectangle Widget::getAbsoluteClippingRectangle() const
  {
    return this->getRelativeClippingRectangleTo(nullptr);
  }

  Rectangle Widget::getRelativeClippingRectangleTo(const Widget* targetAncestor) const
  {
    Rectangle clippingRectangle = this->getClippingRect();
    for (const Widget* widget = this; widget != targetAncestor && widget != nullptr; widget = widget->getParent())
      if (const Widget* widgetParent = widget->getParent())
        clippingRectangle = (clippingRectangle + widgetParent->getChildRelativePosition(widget)) &
                            widgetParent->getClippingRect();
    return clippingRectangle;
  }

  bool Widget::hasParent(Widget* parentToFind) const
  {
    for (const Widget* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
      if (parent == parentToFind)
        return true;
    return false;
  }

  Point Widget::getChildRelativePosition(const Widget* child) const
  {
    assert(child);
    assert(child->getParent() == this);
    return child->location + this->getLeftTopPadding();
  }

  Rectangle Widget::getChildRelativeClippingRectangle(const Widget* child) const
  {
    assert(child);
    assert(child->getParent() == this);
    return child->getClippingRect() + this->getChildRelativePosition(child);
  }

  bool Widget::isTabable() const
  {
    return this->usageBitMask & TABABLE;
  }

  void Widget::setTabable(bool tabable)
  {
    if (tabable)
      this->usageBitMask |= TABABLE;
    else
      this->usageBitMask &= ~TABABLE;
  }

  agui::Widget& Widget::onScaleSetup(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    callback();
    this->actionListeners.emplace_back(owner, Listener::Type::OnGuiScaleSetup);
    this->actionListeners.back().onGuiScaleSetup = std::move(callback);
    return *this;
  }

  Widget& Widget::onClick(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseClick);
    this->actionListeners.back().onMouseClick = std::move(callback);
    return *this;
  }

  Widget& Widget::onClick(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    return this->onClick(owner, [callback = std::move(callback)](const agui::MouseEvent&) { callback(); });
  }

  void Widget::onDoubleClick(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseDoubleClick);
    this->actionListeners.back().onMouseDoubleClick = std::move(callback);
  }

  void Widget::onDoubleClick(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    return this->onDoubleClick(owner, [callback = std::move(callback)](const agui::MouseEvent&) { callback(); });
  }

  void Widget::onToggle(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnToggle);
    this->actionListeners.back().onToggle = [callback](bool leftClick){ (void)(leftClick); callback(); };
  }

  void Widget::onToggle(GenericTargetable* owner, std::function<void(bool leftClick)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnToggle);
    this->actionListeners.back().onToggle = std::move(callback);
  }

  void Widget::onConfirm(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->onConfirm(owner, [callback = std::move(callback)](const agui::KeyEvent&) { callback(); });
  }

  void Widget::onConfirm(GenericTargetable* owner, std::function<void(const agui::KeyEvent& keyEvent)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnConfirm);
    this->actionListeners.back().onConfirm = std::move(callback);
  }

  void Widget::onItemSelectConfirm(GenericTargetable* owner, std::function<void(int index)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemSelectConfirm);
    this->actionListeners.back().onItemSelectConfirm = std::move(callback);
  }

  void Widget::onMouseEnter(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseEnter);
    this->actionListeners.back().onMouseEnter = std::move(callback);
  }

  void Widget::onMouseLeave(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseLeave);
    this->actionListeners.back().onMouseLeave = std::move(callback);
  }

  void Widget::onMouseHover(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseHover);
    this->actionListeners.back().onMouseHover = std::move(callback);
  }

  void Widget::onTextEdit(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnTextEdit);
    this->actionListeners.back().onTextEdit = std::move(callback);
  }

  void Widget::onFocusGain(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnFocusGain);
    this->actionListeners.back().onFocusGain = std::move(callback);
  }

  void Widget::onFocusLose(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnFocusLose);
    this->actionListeners.back().onFocusLose = std::move(callback);
  }

  void Widget::onItemDrag(GenericTargetable* owner, std::function<void(int fromIndex, int toIndex)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemDrag);
    this->actionListeners.back().itemDrag = std::move(callback);
  }

  void Widget::onMouseDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseDown);
    this->actionListeners.back().onMouseDown = std::move(callback);
  }

  void Widget::onMouseUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseUp);
    this->actionListeners.back().onMouseUp = std::move(callback);
  }

  void Widget::onMouseMove(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseMove);
    this->actionListeners.back().onMouseMove = std::move(callback);
  }

  void Widget::onMouseDrag(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseDrag);
    this->actionListeners.back().onMouseDrag = std::move(callback);
  }

  void Widget::onMouseWheelDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseWheelDown);
    this->actionListeners.back().onMouseWheelDown = std::move(callback);
  }

  void Widget::onMouseWheelUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseWheelUp);
    this->actionListeners.back().onMouseWheelUp = std::move(callback);
  }

  void Widget::onMouseWheelLeft(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseWheelLeft);
    this->actionListeners.back().onMouseWheelLeft = std::move(callback);
  }

  void Widget::onMouseWheelRight(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnMouseWheelRight);
    this->actionListeners.back().onMouseWheelRight = std::move(callback);
  }

  void Widget::onModalMouseDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnModalMouseDown);
    this->actionListeners.back().onModalMouseDown = std::move(callback);
  }

  void Widget::onModalMouseUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnModalMouseUp);
    this->actionListeners.back().onModalMouseUp = std::move(callback);
  }

  void Widget::onKeyDown(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnKeyDown);
    this->actionListeners.back().onKeyDown = std::move(callback);
  }

  void Widget::onKeyUp(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnKeyUp);
    this->actionListeners.back().onKeyUp = std::move(callback);
  }

  void Widget::onKeyRepeat(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnKeyRepeat);
    this->actionListeners.back().onKeyRepeat = std::move(callback);
  }

  void Widget::onCenter(GenericTargetable* owner, std::function<void()> callback)
  {
    assert(callback);
    this->actionListeners.emplace_back(owner, Listener::Type::OnCentered);
    this->actionListeners.back().onCenter = std::move(callback);
  }

  Widget& Widget::setFor(Widget* other)
  {
    this->onMouseDown(other, [other](const MouseEvent& mouseEvent) { other->mouseDown(mouseEvent); });
    this->onMouseUp(other, [other](const MouseEvent& mouseEvent) { other->mouseUp(mouseEvent); });
    this->onMouseEnter(other, [other](const MouseEvent& mouseEvent) { other->mouseEnter(mouseEvent); });
    this->onMouseLeave(other, [other](const MouseEvent& mouseEvent) { other->mouseLeave(mouseEvent); });
    return *this;
  }

  agui::Widget& Widget::sharesTooltipWith(Widget* other)
  {
    this->setFor(other);
    this->onMouseHover(other, [other](const MouseEvent& mouseEvent) { other->mouseHover(mouseEvent.copyWithNewSource(other)); });
    this->onMouseMove(other, [other](const MouseEvent& mouseEvent) { other->mouseMove(mouseEvent.copyWithNewSource(other)); });
    return *this;
  }

  Point Widget::createAlignedPosition(AreaAlign alignment, const Rectangle& parentRect, const Dimension& childSize) const
  {
    agui::HorizontalAlign horizontalAlign = agui::toHorizontalAlign(alignment);
    agui::VerticalAlign verticalAlign = agui::toVerticalAlign(alignment);
    int x = 0;
    switch (horizontalAlign)
    {
      case HorizontalAlign::Left: break;
      case HorizontalAlign::Center: x = parentRect.getCenterX() - childSize.width / 2; break;
      case HorizontalAlign::Right: x = parentRect.getRight() - childSize.width; break;
    }

    int y = 0;
    switch (verticalAlign)
    {
      case VerticalAlign::Top: break;
      case VerticalAlign::Center: y = parentRect.getCenterY() - childSize.height / 2; break;
      case VerticalAlign::Bottom: y = parentRect.getBottom() - childSize.height; break;
    }
    return Point(x, y);
  }

  void Widget::addPrivateChild(Widget* widget)
  {
    if (widget == nullptr)
      throw Exception("Cannot add child control because it is nullptr");

    if (widget->parentWidget == nullptr && !containsPrivateChild(widget))
    {
      privateChildren.push_back(widget);

      widget->parentWidget = this;
      assert(widget->guiInstance == nullptr);
    }
  }

  bool Widget::containsPrivateChild(Widget* widget) const
  {
    for (const Widget* child : this->privateChildren)
      if (child == widget)
        return true;
    return false;
  }

  void Widget::removePrivateChild(Widget* widget)
  {
    if (this->removePrivateChildInternal(widget, KeepWidgetAlive::False))
      if (this->getGui())
        this->getGui()->widgetLocationChanged();
  }

  int Widget::getPrivateChildIndex(Widget* widget) const
  {
    // returns index or -1 if not found
    int count = 0;
    for (const Widget* child : this->privateChildren)
    {
      if (child == widget)
        return count;

      count++;
    }
    return -1;
  }

  void Widget::callRecursively(const std::function<void(Widget*)>& callback)
  {
    assert(callback);
    for (Widget* widget : this->privateChildren)
      widget->callRecursively(callback);
    for (Widget* widget : *this)
      widget->callRecursively(callback);
    callback(this);
  }

  bool Widget::dispatchConfirm(const agui::KeyEvent& keyEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnConfirm);
    for (Listener& listener : helper)
      listener.onConfirm(keyEvent);
    return bool(helper);
  }

  bool Widget::dispatchItemSelect(int index, bool leftClick)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnItemSelect);
    for (Listener& listener : helper)
      listener.onItemSelect(index, leftClick);
    return bool(helper);
  }

  bool Widget::dispatchItemSelectConfirm(int index)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnItemSelectConfirm);
    for (Listener& listener : helper)
      listener.onItemSelectConfirm(index);
    return bool(helper);
  }

  bool Widget::dispatchItemDoubleClick(int index)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnItemDoubleClick);
    for (Listener& listener : helper)
      listener.onItemDoubleClick(index);
    return bool(helper);
  }

  bool Widget::dispatchSliderMove(double value)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnSliderMove);
    for (Listener& listener : helper)
      listener.onSliderMove(value);
    return bool(helper);
  }

  bool Widget::dispatchTextEdit()
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnTextEdit);
    for (Listener& listener : helper)
      listener.onTextEdit();
    return bool(helper);
  }

  void Widget::onSizeChanged(Dimension)
  {
    for (Widget* widget = this; widget && widget->clippingRectangle; widget = widget->parentWidget)
      widget->clippingRectangle.reset();
  }

  void Widget::dispatchMouseMove(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseMove);
    this->mouseMove(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseMove(mouseEvent);
  }

  void Widget::dispatchMouseLeave(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseLeave);
    this->mouseLeave(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseLeave(mouseEvent);
  }

  void Widget::dispatchMouseEnter(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseEnter);
    this->mouseEnter(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseEnter(mouseEvent);
  }

  void Widget::dispatchMouseWheelDown(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseWheelDown);
    this->mouseWheelDown(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseWheelDown(mouseEvent);
  }

  void Widget::dispatchMouseWheelUp(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseWheelUp);
    this->mouseWheelUp(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseWheelUp(mouseEvent);
  }

  void Widget::dispatchMouseWheelLeft(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseWheelLeft);
    this->mouseWheelLeft(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseWheelLeft(mouseEvent);
  }

  void Widget::dispatchMouseWheelRight(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseWheelRight);
    this->mouseWheelRight(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseWheelRight(mouseEvent);
  }

  void Widget::dispatchMouseDrag(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseDrag);
    this->mouseDrag(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseDrag(mouseEvent);
  }

  void Widget::dispatchModalMouseDown(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnModalMouseDown);
    this->modalMouseDown(mouseEvent);
    for (Listener& listener : helper)
      listener.onModalMouseDown(mouseEvent);
  }

  void Widget::dispatchMouseDown(const MouseEvent& mouseEvent)
  {
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return;
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseDown);
    this->mouseDown(mouseEvent);

    if ((this->usageBitMask & FIRE_CLICK_ON_MOUSE_DOWN) != 0 && helper)
      this->dispatchClick(mouseEvent.copyWithNewType(MouseEvent::Type::MOUSE_CLICK));

    for (Listener& listener : helper)
      listener.onMouseDown(mouseEvent);
  }

  void Widget::dispatchMouseUp(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseUp);
    this->mouseUp(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseUp(mouseEvent);
  }

  void Widget::dispatchClick(const MouseEvent& mouseEvent)
  {
    if (!this->isIncludedInMouseButtonFilter(mouseEvent.getButton()))
      return;
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseClick);
    this->mouseClick(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseClick(mouseEvent);
  }

  void Widget::dispatchToggle(bool leftClick)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnToggle);
    for (Listener& listener : helper)
      listener.onToggle(leftClick);
  }

  void Widget::dispatchDoubleClick(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseDoubleClick);
    this->mouseDoubleClick(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseDoubleClick(mouseEvent);
  }

  void Widget::dispatchMouseHover(const MouseEvent& mouseEvent)
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnMouseHover);
    this->mouseHover(mouseEvent);
    for (Listener& listener : helper)
      listener.onMouseHover(mouseEvent);
  }

  void Widget::dispatchItemDrag(int fromIndex, int toIndex)
  {
    EventDispatchHelper helper(this, this->actionListeners, agui::Listener::Type::OnItemDrag);
    this->itemDrag(fromIndex, toIndex);
    for (agui::Listener& listener : helper)
      listener.itemDrag(fromIndex, toIndex);
  }

  Rectangle Widget::getRelativeRectangle() const
  {
    return Rectangle(this->getLocation(), this->getSize());
  }

  int Widget::getTextLength() const
  {
    return this->textLen;
  }

  int Widget::updateTextLength()
  {
    int oldTextLen = this->textLen;
    this->textLen = int(UTF8::length(this->getText()));
    if (this->textLen != oldTextLen)
      this->triggerResize();
    return this->textLen;
  }

  void Widget::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (this->isInset() && !forceInFrame)
      return;
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, Rectangle(Point(0, 0), this->getSize())))
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

  void Widget::recursivePaintChildren(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    const Style* style = this->getStyle();
    const Effect* effect = style ? style->getEffect() : nullptr;
    const float effectOpacity = style ? style->getEffectOpacity() : 0;
    if (effect)
      graphicsContext->beginEffect();

#ifdef DEBUG
    ScopedSetter guard1(this->privateChildrenBeingIterated, true);
    ScopedSetter guard2(this->childrenBeingIterated, true);
#endif

    this->recursivePaintChildrenInternal(enabled, graphicsContext, absolutePosition);

    if (effect)
      graphicsContext->endEffect(effect, effectOpacity);
  }

  void Widget::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);
    if (graphicsContext->pushClippingRect(this, this->getSizeRectangle()))
    {
      this->paint(PaintEvent(enabled, graphicsContext), absolutePosition);

      const int leftPadding = this->getLeftPadding();
      const int topPadding = this->getTopPadding();

      for (Widget* widget : this->getPrivateChildren())
        if (widget->isVisible() && widget->shouldRender())
          widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));
      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintChildren(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });
    }

    graphicsContext->popClippingRect();

    if (this->isInset())
      this->recursivePaintShadows(enabled, graphicsContext, absolutePosition, true);
  }

  void Widget::recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    (void)forceInFrame;
    if (enabled)
      enabled = this->isEnabled();

    graphicsContext->setOffset(absolutePosition);

    if (graphicsContext->pushClippingRect(this, Rectangle(Point(0, 0), this->getSize())))
    {
      if (includeThis)
        this->paintBackgroundGlow(PaintEvent(enabled, graphicsContext), absolutePosition);

      const int leftPadding = this->getLeftPadding();
      const int topPadding = this->getTopPadding();

      for (Widget* widget : this->getPrivateChildren())
        if (widget->isVisible() && widget->shouldRender())
          widget->recursivePaintGlows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding));
      drawCulledChildren(*this, graphicsContext, absolutePosition, leftPadding, topPadding, [&](Widget* widget) { widget->recursivePaintGlows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), leftPadding, topPadding)); });
    }
    graphicsContext->popClippingRect();
  }

  Rectangle Widget::getSizeRectangle() const
  {
    return Rectangle(0, 0, getWidth(), getHeight());
  }

  Rectangle Widget::getContentRectangle() const
  {
    int left, right, top, bottom;
    this->getPaddings(&left, &right, &top, &bottom);
    return Rectangle(left, top,
                     this->getWidth() - left - right, this->getHeight() - top - bottom);
  }

  Rectangle Widget::getContentRectangleWithoutPadding() const
  {
    int left = this->getLeftBorder();
    int right = this->getRightBorder();
    int top = this->getTopBorder();
    int bottom = this->getBottomBorder();
    return Rectangle(left, top,
                     this->getWidth() - left - right, this->getHeight() - top - bottom);
  }

  Dimension Widget::getContentSize() const
  {
    int left, right, top, bottom;
    this->getPaddings(&left, &right, &top, &bottom);
    return Dimension(this->getWidth() - left - right, this->getHeight() - top - bottom);
  }

  Rectangle Widget::getContentSizeAsRectangle() const
  {
    int left, right, top, bottom;
    this->getPaddings(&left, &right, &top, &bottom);
    return Rectangle(0, 0, this->getWidth() - left - right, this->getHeight() - top - bottom);
  }

  void Widget::clear()
  {
    if (this->children.empty())
      return;

    assert(!this->childrenBeingIterated);

    WidgetArray removeWidgets = std::move(this->children);
    for (Widget* widget : removeWidgets)
      this->handleChildRemoved(widget);
    if (this->getGui())
      this->getGui()->widgetLocationChanged();
    this->triggerResize();
  }

  void Widget::sendToTop()
  {
    if (Widget* top = this->getTopWidget())
    {
      if (Widget* parent = this->getParent())
      {
        if (parent == top)
        {
          this->bringToFront();
          return;
        }
        parent->remove(this);
      }
      top->add(this);
    }
  }

  bool Widget::modalMouseUp(const MouseEvent&)
  {
    return false;
  }

  void Widget::alignToParent(AreaAlign alignment)
  {
    if (Widget* parent = this->getParent())
      this->setLocation(this->createAlignedPosition(alignment, parent->getContentRectangle(), this->getSize()));
  }

  void Widget::flagForDestruction()
  {
    if (this->isFlaggedForDestruction())
      return;

    this->clear(); // To force instant call of handle removal on parent widgets marked for destructions when removed from this
    this->handleRemoval();
    this->usageBitMask |= FLAGGED_FOR_DESTRUCTION;
    Widget::widgetsToDestroy.widgets.push_back(this);
    this->clearTargetingMeGeneric();
  }

  bool Widget::isFlaggedForDestruction() const
  {
    return this->usageBitMask & FLAGGED_FOR_DESTRUCTION;
  }

  void Widget::flagChildrenForDestruction()
  {
    for (Widget* widget : *this)
      widget->flagForDestruction();
  }

  void Widget::flagAllChildrenForDestruction()
  {
    for (Widget* widget : *this)
    {
      widget->flagForDestruction();
      widget->flagAllChildrenForDestruction();
    }
  }

  void Widget::handleChildRemoved(Widget* child, KeepWidgetAlive keepWidgetAlive)
  {
    if (child->toolTip)
    {
      child->toolTip->flagForDestruction();
      child->toolTip.clear();
    }
    if (this->getGui())
      this->getGui()->dispatchWidgetDestroyed(child);
    child->clearParentWidget(keepWidgetAlive);
  }

  const agui::Widget* Widget::visibleBack() const
  {
    for (auto it = this->children.rbegin(); it != this->children.rend(); ++it)
      if ((*it)->isVisible())
        return *it;
    return nullptr;
  }

  uint32_t Widget::getPrivateChildCount() const
  {
    return uint32_t(this->privateChildren.size());
  }

  Widget* Widget::getPrivateChildAt(uint32_t index) const
  {
    if (index >= this->privateChildren.size())
      return nullptr;

    return this->privateChildren[index];
  }

  CursorProvider::CursorEnum Widget::getEnterCursor() const
  {
    return CursorProvider::DEFAULT_CURSOR;
  }

  bool Widget::setCursor(CursorProvider::CursorEnum cursor)
  {
    if (!this->getGui())
      return false;
    return this->getGui()->setCursor(cursor);
  }

  void Widget::resizeToContents()
  {
    if (this->isKeepSize())
      return;
    int width = 0, height = 0;
    if (Style* style = this->getStyle())
    {
      if (!this->isHorizontallyStretchable())
        width = style->getNaturalWidth();
      if (!this->isVerticallyStretchable())
        height = style->getNaturalHeight();
    }
    else
    {
      // Widgets without style used before styles are initialized should keep their original size.
      width = this->getWidth();
      height = this->getHeight();
    }
    this->setSize(width, height);
  }

  void Widget::resizeToContentsRecursive()
  {
    if (Style* style = this->getStyle())
      if (!this->isResizeToContentsExplicitly())
      {
        int minimalWidth = style->getMinimalWidth();
        if (minimalWidth != 0 && minimalWidth == style->getMaximalWidth())
        {
          int minimalHeight = style->getMinimalHeight();
          if (minimalHeight != 0 && minimalHeight == style->getMaximalHeight())
            return;
        }
      }
    this->clearResizeToContentExplicitly();

    for (Widget* widget : *this)
      widget->resizeToContentsRecursive();
    for (Widget* widget : this->getPrivateChildren())
      widget->resizeToContentsRecursive();

    this->resizeToContents();
    this->sizeBeforeStretching = this->getSize();
  }

  void Widget::applySizeRestrictionsInternal(const Style& style)
  {
    if (this->applySizeRestrictionsInternalWithoutTriggerResize(style))
      this->triggerResize();
  }

  bool Widget::applySizeRestrictionsInternalWithoutTriggerResize(const Style& style)
  {
    int width = this->size.width;
    int height = this->size.height;

    auto maximalWidth = style.getMaximalWidth();
    auto minimalWidth = style.getMinimalWidth();
    if (width < minimalWidth)
      width = minimalWidth;
    if (width > maximalWidth && maximalWidth > 0)
      width = maximalWidth;

    auto maximalHeight = style.getMaximalHeight();
    auto minimalHeight = style.getMinimalHeight();
    if (height < minimalHeight)
      height = minimalHeight;
    if (height > maximalHeight && maximalHeight > 0)
      height = maximalHeight;

    if (width == this->size.width && height == this->size.height)
      return false;

    if (minimalHeight > this->sizeBeforeStretching.height)
      this->sizeBeforeStretching.height = minimalHeight;

    if (minimalWidth > this->sizeBeforeStretching.width)
      this->sizeBeforeStretching.width = minimalWidth;

    this->size.set(width, height);
    if (this->getGui() && this->getGui()->getLockWidget() == nullptr)
      this->getGui()->widgetLocationChanged();
    return true;
  }

  void Widget::applySizeRestrictions(const Style& style)
  {
    Dimension originalSize(this->size);
    this->applySizeRestrictionsInternal(style);
    if (originalSize != this->size)
    {
      this->onSizeChanged(originalSize);
      this->setResizeToContentExplicitly();
    }
  }

  void Widget::setSizeForce(int width, int height, SetSizeInfo setSizeInfo)
  {
    bool dontDecreaseWidthValue = (this->usageBitMask & DONT_DECREASE_WIDTH) != 0;
    this->usageBitMask &= ~DONT_DECREASE_WIDTH;
    bool dontDecreaseHeightValue = (this->usageBitMask & DONT_DECREASE_HEIGHT) != 0;
    this->usageBitMask &= ~DONT_DECREASE_HEIGHT;
    this->setSize(width, height, setSizeInfo);
    if (dontDecreaseWidthValue)
      this->usageBitMask |= DONT_DECREASE_WIDTH;
    if (dontDecreaseHeightValue)
      this->usageBitMask |= DONT_DECREASE_HEIGHT;
  }

  int Widget::getTopPadding() const
  {
    return this->getStyle()->getTopPadding() + this->getTopBorder();
  }

  int Widget::getRightPadding() const
  {
    return this->getStyle()->getRightPadding() + this->getRightBorder();
  }

  int Widget::getBottomPadding() const
  {
    return this->getStyle()->getBottomPadding() + this->getBottomBorder();
  }

  int Widget::getLeftPadding() const
  {
    return this->getStyle()->getLeftPadding() + this->getLeftBorder();
  }

  Point Widget::getLeftTopPadding() const
  {
    return Point(this->getLeftPadding(), this->getTopPadding());
  }

  void Widget::getPaddings(int* left, int* right, int* top, int* bottom) const
  {
    if (left)
      *left = this->getLeftPadding();
    if (right)
      *right = this->getRightPadding();
    if (bottom)
      *bottom = this->getBottomPadding();
    if (top)
      *top = this->getTopPadding();
  }

  int Widget::getTopBorder() const
  {
    if (const ElementImageSet* elementImageSet = this->getBorderImageSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        return elementImageSet->getTopBorder();
    return 0;
  }

  int Widget::getRightBorder() const
  {
    if (const ElementImageSet* elementImageSet = this->getBorderImageSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        return elementImageSet->getRightBorder();
    return 0;
  }

  int Widget::getBottomBorder() const
  {
    if (const ElementImageSet* elementImageSet = this->getBorderImageSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        return elementImageSet->getBottomBorder();
    return 0;
  }

  int Widget::getLeftBorder() const
  {
    if (const ElementImageSet* elementImageSet = this->getBorderImageSet())
      if (elementImageSet->base.drawType == ElementImageSet::Layer::DrawType::Inner)
        return elementImageSet->getLeftBorder();
    return 0;
  }

  void Widget::setContentSize(int width, int height)
  {
    int topPadding, rightPadding, bottomPadding, leftPadding;
    this->getPaddings(&leftPadding, &rightPadding, &topPadding, &bottomPadding);
    this->setSize(width + leftPadding + rightPadding, height + topPadding + bottomPadding);
  }

  void Widget::setContentSizeInternal(int width, int height)
  {
    int topPadding, rightPadding, bottomPadding, leftPadding;
    this->getPaddings(&leftPadding, &rightPadding, &topPadding, &bottomPadding);
    this->setSizeInternal(width + leftPadding + rightPadding, height + topPadding + bottomPadding);
  }

  void Widget::setContentSize(const Dimension& size)
  {
    this->setContentSize(size.width, size.height);
  }

  int Widget::getContentWidth() const
  {
    return std::max(this->getWidth() - this->getHorizontalPaddings(), 0);
  }

  int Widget::getContentHeight() const
  {
    return std::max(this->getHeight() - this->getVerticalPaddings() - this->getAdditionalTopPadding(), 0);
  }

  int Widget::getHorizontalPaddings() const
  {
    return this->getLeftPadding() + this->getRightPadding();
  }

  int Widget::getVerticalPaddings() const
  {
    return this->getTopPadding() + this->getBottomPadding();
  }

  int Widget::getHorizontalMargins() const
  {
    return this->getLeftMargin() + this->getRightMargin();
  }

  int Widget::getVerticalMargins() const
  {
    return this->getTopMargin() + this->getBottomMargin();
  }

  int Widget::getTopMargin() const
  {
    return this->getStyle()->getTopMargin();
  }

  int Widget::getBottomMargin() const
  {
    return this->getStyle()->getBottomMargin();
  }

  int Widget::getLeftMargin() const
  {
    return this->getStyle()->getLeftMargin();
  }

  int Widget::getRightMargin() const
  {
    return this->getStyle()->getRightMargin();
  }

  bool Widget::isHorizontallyStretchable() const
  {
    StretchRule stretchRule = this->getStyle()->isHorizontallyStretchable();
    if (stretchRule != StretchRule::Auto)
      return stretchRule != StretchRule::Off;
    for (const Widget* widget : ConstVisibleChildren(this))
      if (widget->isHorizontallyStretchable())
        return true;
    for (Widget* widget : this->getPrivateChildren())
      if (widget->isVisible() && widget->isHorizontallyStretchable())
        return true;
    return false;
  }

  bool Widget::isVerticallyStretchable() const
  {
    StretchRule stretchRule = this->getStyle()->isVerticallyStretchable();
    if (stretchRule != StretchRule::Auto)
      return stretchRule != StretchRule::Off;
    for (const Widget* widget : ConstVisibleChildren(this))
      if (widget->isVerticallyStretchable())
        return true;
    for (Widget* widget : this->getPrivateChildren())
      if (widget->isVisible() && widget->isVerticallyStretchable())
        return true;
    return false;
  }

  bool Widget::isVerticallySquashable() const
  {
    StretchRule stretchRule = this->getStyle()->isVerticallySquashable();
    if (stretchRule != StretchRule::Auto)
      return stretchRule != StretchRule::Off;
    for (Widget* widget : *this)
      if (widget->isVisible() && widget->isVerticallySquashable())
        return true;
    for (Widget* widget : this->getPrivateChildren())
      if (widget->isVisible() && widget->isVerticallySquashable())
        return true;
    return false;
  }

  int Widget::maximumVerticalSquashSize() const
  {
    StretchRule squashRule = this->getStyle()->isVerticallySquashable();
    if (squashRule == StretchRule::On)
      return this->getHeight() - this->getStyle()->getMinimalHeight();
    return std::max(0, this->getHeight() - this->getSizeBeforeStretching().height);
  }

  int Widget::maximumHorizontalSquashSize() const
  {
    StretchRule squashRule = this->getStyle()->isHorizontallySquashable();
    if (squashRule == StretchRule::On)
      return this->getWidth() - this->getStyle()->getMinimalWidth();
    return std::max(0, this->getWidth() - this->getSizeBeforeStretching().width);
  }

  bool Widget::isHorizontallySquashable() const
  {
    int maximalWidth = this->getStyle()->getMaximalWidth();
    if (maximalWidth > 0 &&
        maximalWidth == this->getStyle()->getMinimalWidth())
      return false;
    StretchRule stretchRule = this->getStyle()->isHorizontallySquashable();
    if (stretchRule != StretchRule::Auto)
      return stretchRule != StretchRule::Off;
    for (const Widget* widget : ConstVisibleChildren(this))
      if (widget->isHorizontallySquashable())
        return true;
    for (Widget* widget : this->getPrivateChildren())
      if (widget->isVisible() && widget->isHorizontallySquashable())
        return true;
    return false;
  }

  void Widget::displaySizeChangedRecursive()
  {
    for (Widget* widget : *this)
      widget->displaySizeChangedRecursive();
    for (Widget* widget : this->getPrivateChildren())
      widget->displaySizeChangedRecursive();
    this->displaySizeChanged();
  }

  void Widget::postDisplaySizeChangedRecursive()
  {
    for (Widget* widget : *this)
      widget->postDisplaySizeChangedRecursive();
    for (Widget* widget : this->getPrivateChildren())
      widget->postDisplaySizeChangedRecursive();
    this->postDisplaySizeChanged();
  }

  void Widget::displaySizeChanged()
  {
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnGuiScaleSetup))
      listener.onGuiScaleSetup();
    if (Style* style = this->getStyle())
      this->applySizeRestrictions(*style);
  }

  void Widget::postDisplaySizeChanged()
  {
    if (Style* style = this->getStyle())
      this->applySizeRestrictions(*style);
  }

  void Widget::flagsChanged()
  {
    this->triggerResize();
  }

  agui::Widget* Widget::getWidgetUnderMouse(Point mousePosition,
                                            const Point thisAbsolutePosition,
                                            agui::Rectangle absolutePositionClip,
                                            TransparentValue parentTransparent)
  {
    Point contentAreaAbsolutePosition = thisAbsolutePosition + this->getLeftTopPadding();
    const Point intersectionPoint = mousePosition - contentAreaAbsolutePosition;
    TransparentValue transparent = transparentOpaqueMask(this->isTransparent(), parentTransparent);

    for (WidgetArray* children : {&this->getChildren(), &this->getPrivateChildren()})
      for (auto childIt = children->rbegin(); childIt != children->rend(); childIt++)
      {
        Widget* child = *childIt;
        if (child->isUnderMouse(intersectionPoint))
          if (Widget* underMouse = child->getWidgetUnderMouse(mousePosition,
                                                              this->getChildRelativePosition(child) + thisAbsolutePosition,
                                                              absolutePositionClip & (this->getChildRelativeClippingRectangle(child) +
                                                                                      thisAbsolutePosition),
                                                              transparent))
            return underMouse;
      }

    return this->getUnderMouseWithTransparency(transparent);
  }

  Widget* Widget::getUnderMouseWithTransparency(TransparentValue transparent)
  {
    if (transparent != TransparentValue::No) // If it's 'DependsOnChildren' one of those would be under the mouse.
      return nullptr;
    return this;
  }

  agui::Widget& Widget::squashHorizontally()
  {
    this->getStyle()->setHorizontallySquashable();
    return *this;
  }

  agui::Widget& Widget::dontSquashHorizontally()
  {
    this->getStyle()->setHorizontallySquashable(false);
    return *this;
  }

  agui::Widget& Widget::squashVertically()
  {
    this->getStyle()->setVerticallySquashable();
    return *this;
  }

  agui::Widget& Widget::dontSquashVertically()
  {
    this->getStyle()->setVerticallySquashable(false);
    return *this;
  }

  agui::Widget& Widget::squash()
  {
    this->getStyle()->setHorizontallySquashable();
    this->getStyle()->setVerticallySquashable();
    return *this;
  }

  agui::Widget& Widget::stretchHorizontally()
  {
    this->getStyle()->setHorizontallyStretchable();
    return *this;
  }

  agui::Widget& Widget::stretchAndExpandHorizontally()
  {
    this->getStyle()->setHorizontallyStretchable(StretchRule::StretchAndExpand);
    return *this;
  }

  agui::Widget& Widget::dontStretchHorizontally()
  {
    this->getStyle()->setHorizontallyStretchable(StretchRule::Off);
    return *this;
  }

  agui::Widget& Widget::stretchVertically()
  {
    this->getStyle()->setVerticallyStretchable();
    return *this;
  }

  agui::Widget& Widget::stretchAndExpandVertically()
  {
    this->getStyle()->setVerticallyStretchable(StretchRule::StretchAndExpand);
    return *this;
  }

  agui::Widget& Widget::dontStretchVertically()
  {
    this->getStyle()->setVerticallyStretchable(StretchRule::Off);
    return *this;
  }

  agui::Widget& Widget::stretch()
  {
    this->getStyle()->setHorizontallyStretchable();
    this->getStyle()->setVerticallyStretchable();
    return *this;
  }

  agui::Widget& Widget::dontStretch()
  {
    this->getStyle()->setHorizontallyStretchable(StretchRule::Off);
    this->getStyle()->setVerticallyStretchable(StretchRule::Off);
    return *this;
  }

  agui::Widget& Widget::setLeftMargin(int16_t margin)
  {
    this->getStyle()->setLeftMargin(margin);
    return *this;
  }

  agui::Widget& Widget::setRightMargin(int16_t margin)
  {
    this->getStyle()->setRightMargin(margin);
    return *this;
  }

  agui::Widget& Widget::setTopMargin(int16_t margin)
  {
    this->getStyle()->setTopMargin(margin);
    return *this;
  }

  agui::Widget& Widget::setBottomMargin(int16_t margin)
  {
    this->getStyle()->setBottomMargin(margin);
    return *this;
  }

  agui::Widget& Widget::setVerticalAlign(agui::VerticalAlign alignment)
  {
    this->getStyle()->setVerticalAlign(alignment);
    return *this;
  }

  agui::Widget& Widget::setHorizontalAlign(agui::HorizontalAlign alignment)
  {
    this->getStyle()->setHorizontalAlign(alignment);
    return *this;
  }

  void Widget::setFlushToTopOfStyle(const agui::Style& style)
  {
    this->getStyle()->setLeftMargin(-style.getLeftPadding());
    this->getStyle()->setTopMargin(-style.getTopPadding());
    this->getStyle()->setRightMargin(-style.getRightPadding());
  }

  void Widget::setFlushToBottomOfStyle(const agui::Style& style)
  {
    this->getStyle()->setLeftMargin(-style.getLeftPadding());
    this->getStyle()->setRightMargin(-style.getRightPadding());
    this->getStyle()->setBottomMargin(-style.getBottomPadding());
  }

  bool Widget::isVisible() const
  {
    return (this->usageBitMask & VISIBLE) != 0 && (this->usageBitMask & HIDDEN_BY_SEARCH) == 0;
  }

  void Widget::setVisible(bool visible)
  {
    if (this->setVisibleSilent(visible))
      this->triggerResize();
  }

  bool Widget::setVisibleSilent(bool visible)
  {
    if (this->isVisible() == visible)
      return false;
    if (visible)
      this->usageBitMask |= VISIBLE;
    else
      this->usageBitMask &= ~VISIBLE;
    if (!visible)
      this->checkLostFocusRecursive();
    return true;
  }

  bool Widget::hasVisibleChild() const
  {
    return const_visible_iterator(*this) != this->end();
  }

  void Widget::keepVerticallyVisibleInScrollPane(int verticalPosition, int margin)
  {
    // here we need to take into account that the position of this widget within
    // its parent might not be at vertical position 0 (hence adding vertical location)
    if (Widget* parent = this->getParent())
      parent->keepVerticallyVisibleInScrollPane(verticalPosition  + this->getLocation().y + this->getParent()->getTopPadding(), margin);
  }

  agui::ToolTip* Widget::getToolTip()
  {
    return *this->toolTip;
  }

  const ToolTip* Widget::getToolTip() const
  {
    return *this->toolTip;
  }

  ToolTip* Widget::checkCreateStyleTooltip()
  {
    if (this->getStyle())
    {
      ToolTip* resultToolTip = new ToolTip(agui::GuiDirection::Horizontal, &ToolTip::defaultToolTipOuterStyle);

      agui::Frame& widgetInfoPart = agui::frame(&agui::ToolTip::defaultToolTipStyle);
      widgetInfoPart << agui::label(Util::demangle(typeid(*this).name()), &ToolTip::defaultTitleStyle);
      this->createWidgetInfoTooltipContents(static_cast<agui::VerticalFlow&>(*widgetInfoPart.layout));

      agui::Frame& styleInfoPart = agui::frame(&agui::ToolTip::defaultToolTipStyle);
      this->createStyleTooltipContents(static_cast<agui::VerticalFlow&>(*styleInfoPart.layout));
      *resultToolTip << widgetInfoPart << styleInfoPart;
      return resultToolTip;
    }
    return nullptr;
  }

  void Widget::createStyleTooltipContents(VerticalFlow& result)
  {
    const Style* style = this->getStyle();
    style->addChangedValues(result, style);
    if (const Style* parent = style->getParent())
    {
      agui::HorizontalFlow& line = agui::hFlow;
      line << agui::label("Style:");
      if (parent->styleInfo)
        if (!parent->styleInfo->name.empty())
          line << agui::label(parent->styleInfo->name, Gui::instance->styleNameLabelStyle);
        else
        {
          std::string partOfStyle = "unknown";
          const Style* currentDefined = parent;
          while (currentDefined &&
                  currentDefined->styleInfo &&
                  currentDefined->styleInfo->name.empty() &&
                  currentDefined->styleInfo->styleThatDefinesThis &&
                  currentDefined->styleInfo->styleThatDefinesThis->styleInfo)
            currentDefined = currentDefined->styleInfo->styleThatDefinesThis;
          if (currentDefined && currentDefined->styleInfo && !currentDefined->styleInfo->name.empty())
            partOfStyle = currentDefined->styleInfo->name;
          line << agui::label("Part of") << agui::label(partOfStyle, Gui::instance->styleNameLabelStyle) << agui::label("definition");
        }
      result << line;
      parent->addChangedValues(result, style);
      while (parent->getParent())
      {
        parent = parent->getParent();
        std::string name = std::string((parent->styleInfo && !parent->styleInfo->name.empty()) ? parent->styleInfo->name : "unknown");
        result << (agui::hFlow << agui::label("Derived from:") << agui::label(std::move(name), Gui::instance->styleNameLabelStyle));
        parent->addChangedValues(result, style);
      }
    }
  }

  void Widget::createWidgetInfoTooltipContents(VerticalFlow& result) const
  {
    this->addWidgetInfoComment(result, "relative", ssprintf("[%i, %i]", this->getLocation().x, this->getLocation().y));
    this->addWidgetInfoComment(result, "size", ssprintf("{%d, %d}", this->getWidth(), this->getHeight()));
    this->addWidgetInfoComment(result, "content_size", ssprintf("{%d, %d}", this->getContentWidth(), this->getContentHeight()));
    this->addWidgetInfoComment(result, "clip_size", this->clippingRectangle ? this->clippingRectangle->str().c_str() : "<null>");
    this->addWidgetInfoComment(result, "size_before_stretching", ssprintf("{%d, %d}", this->sizeBeforeStretching.width, this->sizeBeforeStretching.height));
    if (!this->isEnabled())
      this->addWidgetInfoComment(result, "enabled", "false");
    if (!this->shouldRender())
      this->addWidgetInfoComment(result, "render", "false");
    this->addWidgetInfoComment(result, "maximum_horizontal_squash_size", ssprintf("%d", this->maximumHorizontalSquashSize()));
    this->addWidgetInfoComment(result, "maximum_vertical_squash_size", ssprintf("%d", this->maximumVerticalSquashSize()));

    if (!this->children.empty() || !this->privateChildren.empty())
    {
      result << agui::label("children:");
      agui::VerticalFlow& list = agui::vFlow;
      list.style.setLeftPadding(8);

      auto addChild = [&] (const Widget* child)
      {
        this->addWidgetInfoComment(list, Util::demangle(typeid(*child).name()),
                                   ssprintf("%i x %i", child->getWidth(), child->getHeight()));
      };
      for (const Widget* child : this->children)
        addChild(child);
      for (const Widget* child : this->privateChildren)
        addChild(child);

      result << list;
    }
  }

  void Widget::addWidgetInfoComment(VerticalFlow& result, const std::string& caption, const std::string& value) const
  {
    result << (agui::hFlow << agui::label(caption + ":") << agui::label(value));
  }

  bool Widget::removeToolTipWidget(bool forceDelete)
  {
    if (!this->toolTip)
      return false;
    if (!forceDelete)
    {
      this->toolTip->flagForDestruction();
      this->toolTip.clear();
    }
    else
      delete *this->toolTip;
    return true;
  }

  void Widget::updateTooltipPosition(const Point& mousePosition)
  {
    if (!this->toolTip)
      return;

    Point absolutePosition = this->getAbsolutePosition();
    this->toolTip->updatePosition(Point(absolutePosition.x + mousePosition.x,
                                        absolutePosition.y + mousePosition.y));
  }

  void Widget::checkCreateTooltip()
  {
    if (this->toolTip)
    {
      if (this->toolTip->getGui() == nullptr)
        this->getGui()->addToolTip(*this->toolTip);
      return;
    }
    if (Gui::instance->styleView && this->interactsWithStyleView())
    {
      if (!this->toolTipCreator || !this->toolTipCreator->isExtendedStyleViewToolTip())
        this->toolTip = this->checkCreateStyleTooltip();
      else
        this->toolTip = this->toolTipCreator->createToolTip();
      if (Gui::instance->currentStyleTooltip)
        Gui::instance->currentStyleTooltip->flagForDestruction();
      Gui::instance->currentStyleTooltip = this->toolTip;
    }
    else
    {
      this->toolTip = this->toolTipCreator ? this->toolTipCreator->createToolTip() : this->createToolTip();
      if (this->toolTip)
        this->toolTip->updateContent();
      else
        if (this->isEnabled())
          if (const Style* style = this->getStyle())
            if (const std::string* tooltipFromStyle = style->getToolTip())
            {
              this->setToolTip(*tooltipFromStyle);
              this->checkCreateTooltip();
            }
    }
  }

  void Widget::setToolTipCreator(ToolTipCreatorBase* creator)
  {
    bool toolTipExisted = bool(this->toolTip);
    this->removeToolTipWidget();
    delete this->toolTipCreator;
    this->toolTipCreator = creator;
    if (toolTipExisted)
      this->checkCreateTooltip();
  }

  Widget& Widget::setToolTipWithInfoIcon(const std::string& text)
  {
    this->setToolTip(text);
    if (!text.empty())
      this->setText(this->getText() + " " + Gui::instance->infoString);
    return *this;
  }

  Widget& Widget::setToolTipWithInfoIcon(std::string&& text)
  {
    bool empty = text.empty();
    this->setToolTip(std::move(text));
    if (!empty && this->getText().find(Gui::instance->infoString) == std::string::npos)
      this->setText(this->getText() + " " + Gui::instance->infoString);
    return *this;
  }

  Widget& Widget::setToolTip(const std::string& text)
  {
    // Don't update the tooltip creator if it matches the text already
    if (!text.empty() && this->toolTipCreator)
      if (PlainToolTipCreator* plainToolTipCreator = dynamic_cast<PlainToolTipCreator*>(this->toolTipCreator))
        if (plainToolTipCreator->getTitle() == text && plainToolTipCreator->getText().empty())
          return *this;

    this->setToolTipCreator(text.empty() ? nullptr : new PlainToolTipCreator(text));
    return *this;
  }

  Widget& Widget::setToolTip(std::string&& text)
  {
    // Don't update the tooltip creator if it matches the text already
    if (!text.empty() && this->toolTipCreator)
      if (PlainToolTipCreator* plainToolTipCreator = dynamic_cast<PlainToolTipCreator*>(this->toolTipCreator))
        if (plainToolTipCreator->getTitle() == text && plainToolTipCreator->getText().empty())
          return *this;
    this->setToolTipCreator(text.empty() ? nullptr : new PlainToolTipCreator(std::move(text)));
    return *this;
  }

  Widget& Widget::setToolTip(const std::string& title, const std::string& text)
  {
    this->setToolTipCreator(new PlainToolTipCreator(title, text));
    return *this;
  }

  Widget& Widget::setToolTip(std::string&& title, std::string&& text)
  {
    this->setToolTipCreator(new PlainToolTipCreator(std::move(title), std::move(text)));
    return *this;
  }

  Widget& Widget::appendTooltip(std::string&& text)
  {
    if (this->toolTipCreator)
      if (PlainToolTipCreator* plainToolTipCreator = dynamic_cast<PlainToolTipCreator*>(this->toolTipCreator))
      {
        plainToolTipCreator->appendTitle("\n");
        plainToolTipCreator->appendTitle(std::move(text));
        return *this;
      }
    this->setToolTipCreator(new PlainToolTipCreator(std::move(text)));
    return *this;
  }

  Widget& Widget::operator<<(const Empty&)
  {
    EmptyWidget* emptyWidget = new EmptyWidget();
    this->add(emptyWidget);
    emptyWidget->autoDestructWhenRemovedFromParent();
    return *this;
  }

  Widget& Widget::operator<<(const HorizontalPusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->style.setHorizontallyStretchable();
    pusher->ignoreBySearch();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  agui::Widget& Widget::operator<<(const VerticalPusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->ignoreBySearch();
    pusher->style.setVerticallyStretchable();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  void Widget::triggerResize()
  {
    if (this->parentWidget == nullptr)
      return;
    // ::setResizeToContentExplicitly() needs to be set regardless of the parent/top-parent being in a GUI.
    // Otherwise, when the parent is eventually put into a GUI, the child elements will not be sized correctly.
    this->setResizeToContentExplicitly();
    if (this->getGui() == nullptr)
      return;
    if (this->parentWidget == this->getGui()->getTop())
      this->getGui()->getTop()->resizeNextTime(this);
    else
      this->parentWidget->triggerResize();
  }

  Widget& Widget::dontDecreaseWidth()
  {
    this->usageBitMask |= DONT_DECREASE_WIDTH;
    return *this;
  }

  Widget& Widget::allowDecreaseWidth()
  {
    this->usageBitMask &= ~DONT_DECREASE_WIDTH;
    return *this;
  }

  Widget& Widget::dontDecreaseHeight()
  {
    this->usageBitMask |= DONT_DECREASE_HEIGHT;
    return *this;
  }

  Widget& Widget::allowDecreaseHeight()
  {
    this->usageBitMask &= ~DONT_DECREASE_HEIGHT;
    return *this;
  }

  agui::Widget& Widget::dontDecreaseSize()
  {
    return this->dontDecreaseWidth(), this->dontDecreaseHeight();
  }

  agui::Widget& Widget::allowDecreaseSize()
  {
    return this->allowDecreaseWidth(), this->allowDecreaseHeight();
  }

  agui::Widget& Widget::setAllowDecreaseSize(bool allow)
  {
    return allow ? this->allowDecreaseSize() : this->dontDecreaseSize();
  }

  void Widget::setRender(bool value)
  {
    if (value)
      this->usageBitMask |= RENDER;
    else
    {
      this->usageBitMask &= ~RENDER;
      if (this->isFocused())
        this->clearFocus();
    }
  }

  TransparentValue transparentOpaqueMask(TransparentValue value, TransparentValue mask)
  {
    if (mask == TransparentValue::No)
      return TransparentValue::No;
    return value;
  }

  Label& label(const std::string& text, const LabelStyle* parentStyle, SingleLine singleLine)
  {
    return label(std::string(text), parentStyle, singleLine);
  }

  Label& label(const std::string& text, SingleLine singleLine)
  {
    return label(text, nullptr, singleLine);
  }

  Label& label(std::string&& text, const LabelStyle* parentStyle, SingleLine singleLine)
  {
    Label* label = parentStyle ? new Label(std::move(text), parentStyle) : new Label(std::move(text));
    if (singleLine != SingleLine::Derive)
      label->setSingleLine(singleLine == SingleLine::True);
    label->autoDestructWhenRemovedFromParent();
    return *label;
  }

  Label& label(std::string&& text, SingleLine singleLine)
  {
    return label(std::move(text), nullptr, singleLine);
  }

  Label& label(const std::string& text, RichTextSetting richTextSetting)
  {
    Label* label = new Label(text, richTextSetting);
    label->autoDestructWhenRemovedFromParent();
    return *label;
  }

  Label& label(std::string&& text, RichTextSetting richTextSetting)
  {
    Label* label = new Label(std::move(text), richTextSetting);
    label->autoDestructWhenRemovedFromParent();
    return *label;
  }

  agui::Label& labelWithToolTip(const std::string& text, const std::string& toolTipText, const LabelStyle* parentStyle, SingleLine singleLine)
  {
    Label& label = agui::label(text, parentStyle, singleLine);
    label.setToolTip(toolTipText);
    return label;
  }

  agui::Label& labelWithToolTip(std::string&& text, std::string&& toolTipText, const LabelStyle* parentStyle, SingleLine singleLine)
  {
    Label& label = agui::label(std::move(text), parentStyle, singleLine);
    label.setToolTip(toolTipText);
    return label;
  }

  agui::Label& labelWithToolTipWithInfoIcon(std::string&& text, std::string&& toolTipText, const LabelStyle* parentStyle, SingleLine singleLine)
  {
    Label& label = agui::label(std::move(text), parentStyle, singleLine);
    if (!toolTipText.empty())
      label.setToolTipWithInfoIcon(std::move(toolTipText));
    return label;
  }

  ImageWidget& image(std::unique_ptr<Image> image, const ImageStyle* parentStyle)
  {
    ImageWidget* imageWidget = new ImageWidget(std::move(image), parentStyle ? parentStyle : &ImageWidget::defaultStyle);
    imageWidget->autoDestructWhenRemovedFromParent();
    return *imageWidget;
  }

  ImageWidget& image(const ImageStyle* parentStyle)
  {
    ImageWidget* imageWidget = new ImageWidget(parentStyle ? parentStyle : &ImageWidget::defaultStyle);
    imageWidget->autoDestructWhenRemovedFromParent();
    return *imageWidget;
  }

  Table& table(int columnCount, const agui::TableStyle* tableStyle)
  {
    Table* table = tableStyle == nullptr ? new Table() : new Table(tableStyle);;
    table->setColumnCount(columnCount);
    table->autoDestructWhenRemovedFromParent();
    return *table;
  }

  agui::Frame& frame(GuiDirection guiDirection, const FrameStyle* parentStyle)
  {
    Frame* frame = new Frame(guiDirection, parentStyle);
    frame->autoDestructWhenRemovedFromParent();
    return *frame;
  }

  agui::Frame& frame(const FrameStyle* parentStyle)
  {
    return frame(GuiDirection::Vertical, parentStyle);
  }

  agui::Frame& frame(std::string&& text, const FrameStyle* parentStyle)
  {
    Frame* frame = new Frame(GuiDirection::Vertical, parentStyle, std::move(text));
    frame->autoDestructWhenRemovedFromParent();
    return *frame;
  }

  agui::Tab& tab(const std::string& text, const TabStyle* parentStyle)
  {
    return *agui::hold(new Tab(text, parentStyle));
  }

  agui::CheckBox& checkBox(bool checked, std::string&& text, Widget* callbackHolder, std::function<void(bool)> callback)
  {
    assert(callback);
    CheckBox* checkBox = new CheckBox();
    checkBox->setText(std::move(text));
    checkBox->setChecked(checked);
    checkBox->onCheckChange(callbackHolder, callback);
    return agui::hold(*checkBox);
  }

  CheckBox& checkBoxWithTooltipWithInfoIcon(bool checked, std::string&& text, std::string&& tooltip, Widget* callbackHolder, std::function<void(bool)> callback)
  {
    assert(callback);
    CheckBox& checkBox = agui::checkBox(checked, std::move(text), callbackHolder, std::move(callback));
    checkBox.setToolTipWithInfoIcon(std::move(tooltip));
    return checkBox;
  }

  agui::Button& button(std::string&& text, Widget* callbackHolder, std::function<void()> callback, const ButtonStyle* style)
  {
    Button* button = new TextButton();
    button->setText(std::move(text));
    if (callback)
      button->onClick(callbackHolder, callback);
    if (style)
      button->style.setParent(style);
    return *agui::hold(button);
  }

  agui::EmptyWidget& filler(const EmptyWidgetStyle* parentStyle)
  {
    EmptyWidget* widget = new EmptyWidget(parentStyle ? parentStyle : &EmptyWidget::defaultStyle);
    widget->autoDestructWhenRemovedFromParent();
    return *widget;
  }

  agui::VerticalLine& verticalLine(const LineStyle* parentStyle)
  {
    VerticalLine* line = new VerticalLine(parentStyle ? parentStyle : &Line::defaultStyle);
    line->autoDestructWhenRemovedFromParent();
    return *line;
  }

  agui::HorizontalFlow& HFlow::operator()(VerticalAlign align) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    horizontalFlow->style.setVerticalAlign(align);
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator()(HorizontalAlign align) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    horizontalFlow->style.setHorizontalAlign(align);
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator()(AreaAlign align) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    horizontalFlow->style.setHorizontalAlign(toHorizontalAlign(align));
    horizontalFlow->style.setVerticalAlign(toVerticalAlign(align));
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator()(const HorizontalFlowStyle* style) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow(style);
    horizontalFlow->autoDestructWhenRemovedFromParent();
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator<<(agui::Widget& widget) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    *horizontalFlow << widget;
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator<<(Widget* widget) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    if (widget)
      *horizontalFlow << widget;;
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::operator<<(const Pusher& pusher) const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    *horizontalFlow << pusher;
    return *horizontalFlow;
  }

  agui::HorizontalFlow& HFlow::centerVertically() const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    return horizontalFlow->centerVertically();
  }

  agui::HorizontalFlow& HFlow::stretchHorizontally() const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    return horizontalFlow->stretchHorizontally();
  }

  agui::HorizontalFlow& HFlow::dontShrinkInReactionToSetSize() const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    return horizontalFlow->dontShrinkInReactionToSetSize();
  }

  agui::HFlow::operator HorizontalFlow&() const
  {
    HorizontalFlow* horizontalFlow = new HorizontalFlow();
    horizontalFlow->autoDestructWhenRemovedFromParent();
    return *horizontalFlow;
  }

  Widget::WidgetsToDestroy Widget::widgetsToDestroy;

  Widget::WidgetsToDestroy::~WidgetsToDestroy()
  {
    this->destroyWidgets();
  }

  void Widget::WidgetsToDestroy::destroyWidgets()
  {
    std::vector<Widget*> toDestroy;
    do
    {
      std::swap(toDestroy, this->widgets);
      for (Widget* widget : toDestroy)
        delete widget;
      toDestroy.clear();
    }
    while (!this->widgets.empty());
  }

  VerticalFlow& VFlow::operator()(VerticalAlign align) const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    verticalFlow->style.setVerticalAlign(align);
    return *verticalFlow;
  }

  VerticalFlow& VFlow::operator()(AreaAlign align) const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    verticalFlow->style.setHorizontalAlign(toHorizontalAlign(align));
    verticalFlow->style.setVerticalAlign(toVerticalAlign(align));
    return *verticalFlow;
  }

  VerticalFlow& VFlow::operator()(const VerticalFlowStyle* parentStyle) const
  {
    VerticalFlow* verticalFlow = new VerticalFlow(parentStyle);
    verticalFlow->autoDestructWhenRemovedFromParent();
    return *verticalFlow;
  }

  VerticalFlow& VFlow::operator<<(Widget& widget) const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    *verticalFlow << widget;
    return *verticalFlow;
  }

  agui::VerticalFlow& VFlow::dontDecreaseWidth() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    verticalFlow->dontDecreaseWidth();
    return *verticalFlow;
  }

  agui::VerticalFlow& VFlow::dontDecreaseHeight() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    verticalFlow->dontDecreaseHeight();
    return *verticalFlow;
  }

  agui::VerticalFlow& VFlow::dontDecreaseSize() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    verticalFlow->dontDecreaseWidth();
    verticalFlow->dontDecreaseHeight();
    return *verticalFlow;
  }

  agui::VerticalFlow& VFlow::centerVertically() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    return verticalFlow->centerVertically();
  }

  agui::VerticalFlow& VFlow::stretchHorizontally() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    return verticalFlow->stretchHorizontally();
  }

  agui::VerticalFlow& VFlow::dontShrinkInReactionToSetSize() const
  {
    VerticalFlow* verticalFlow = new VerticalFlow();
    verticalFlow->autoDestructWhenRemovedFromParent();
    return verticalFlow->dontShrinkInReactionToSetSize();
  }

  VFlow::operator VerticalFlow& () const
  {
    return *agui::hold(new VerticalFlow());
  }

  agui::HorizontalLine& HorizontalLineHelper::operator()(const LineStyle* parentStyle)
  {
    return *agui::hold(new HorizontalLine(parentStyle ? parentStyle : &Line::defaultStyle));
  }

  HorizontalLineHelper::operator HorizontalLine& () const
  {
    return *agui::hold(new HorizontalLine());
  }

  VerticalScrollPane& VScroll::operator<<(agui::Widget& widget) const
  {
    VerticalScrollPane* verticalScrollPane = new VerticalScrollPane();
    verticalScrollPane->autoDestructWhenRemovedFromParent();
    *verticalScrollPane << widget;
    return *verticalScrollPane;
  }

  VerticalScrollPane& VScroll::operator()(const ScrollPaneStyle* parentStyle) const
  {
    VerticalScrollPane* scrollPane = new VerticalScrollPane(parentStyle);
    scrollPane->autoDestructWhenRemovedFromParent();
    return *scrollPane;
  }

  VScroll::operator VerticalScrollPane&() const
  {
    VerticalScrollPane* verticalScrollPane = new VerticalScrollPane();
    verticalScrollPane->autoDestructWhenRemovedFromParent();
    return *verticalScrollPane;
  }

  agui::VerticalScrollPane& VScroll::stretchHorizontally() const
  {
    agui::VerticalScrollPane& result = agui::vScroll;
    result.stretchHorizontally();
    return result;
  }

  bool Widget::isIncludedInMouseButtonFilter(MouseButton button) const
  {
    if (!this->isEnabled() && !this->isReactWhenDisabled())
      return false;
    return (uint16_t(this->mouseButtonFilter) & uint16_t(button)) != 0;
  }

  void Widget::setIgnoredByInteraction(bool value)
  {
    if (value)
      this->usageBitMask |= IGNORED_BY_INTERACTION;
    else
      this->usageBitMask &= ~IGNORED_BY_INTERACTION;
  }

  void Widget::setKeepSize(bool value)
  {
    if (value)
      this->usageBitMask |= KEEP_SIZE;
    else
      this->usageBitMask &= ~KEEP_SIZE;
  }

  bool Widget::canBeHiddenBySearch() const
  {
    if (!this->getStyle()->canBeHiddenBySearch())
      return false;
    for (Widget* widget : this->getPrivateChildren())
      if (!widget->canBeHiddenBySearch())
        return false;
    for (Widget* widget : *this)
      if (!widget->canBeHiddenBySearch())
        return false;
    return true;
  }

  Widget& Widget::setStyle(const Style* newStyle)
  {
    if (Style* style = this->getStyle())
      style->setParent(newStyle);
    return *this;
  }

  FocusManager* Widget::getFocusManager() const
  {
    if (Gui* gui = this->getGui())
      return &gui->focusManager;
    return nullptr;
  }
}

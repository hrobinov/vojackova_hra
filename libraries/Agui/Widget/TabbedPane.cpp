#include "Agui/Gui.hpp"
#include "Agui/Widget/HorizontalFlow.hpp"
#include "Agui/Widget/TabbedPane.hpp"
#include "Agui/Widget/Tab.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include <algorithm>
#include <Agui/EventDispatchHelper.hpp>

namespace agui
{
  TabbedPaneStyle TabbedPane::defaultStyle;

  TabbedPane::TabbedPane(const TabbedPaneStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &TabbedPane::defaultStyle)
    , tabContainer(uint32_t(20 /* the default amount of slots per row*/), this->style.getTabContainerStyle())
    , contentFrame(GuiDirection::Horizontal, parentStyle ? parentStyle->getContentFrame() : TabbedPane::defaultStyle.getContentFrame())
    , selectedTab(this->tabs.end())
  {
    this->addPrivateChild(&this->tabContainer);
    this->addPrivateChild(&this->contentFrame);
  }

  TabbedPane::~TabbedPane()
  {
    this->clearTabs();
  }

  void TabbedPane::reapplySubStyles()
  {
    this->tabContainer.style.setParent(this->style.getTabContainerStyle());
    this->contentFrame.style.setParent(this->style.getContentFrame());
  }

  void TabbedPane::clear()
  {
    this->clearTabs();
    super::clear();
  }

  void TabbedPane::clearTabs()
  {
    while (!this->tabs.empty())
      this->removeTab(int(this->tabs.size() - 1));
  }

  void TabbedPane::setResizeToCurrentTabContents(bool value)
  {
    this->resizeToCurrentTabContents = value;
    this->triggerResize();
  }

  bool TabbedPane::genericSearch(const LowercaseString& filter)
  {
    // This calls for future extension of this logic, to highlight tabs that contain something matching the search.
    // It is not needed for anything yet.
    for (auto& item : this->tabs)
      item.content->genericSearch(filter);
    return true;
  }

  void TabbedPane::onSelectedTabChange(GenericTargetable* owner, std::function<void()> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::SelectedTabChanged);
    this->actionListeners.back().onSelectedTabChange = std::move(callback);
  }

  void TabbedPane::adjustWidgetContainer()
  {
    this->contentFrame.setSize(this->getWidestContentTab() + this->getHorizontalPaddings() + this->contentFrame.getHorizontalPaddings(),
                               this->getHighestContentTab() + this->contentFrame.getVerticalPaddings());
    if (this->tabContainer.isVisible())
      this->contentFrame.setLocation(0, this->tabContainer.getHeight());
    else
      this->contentFrame.setLocation(0, 0);
  }

  void TabbedPane::addTab(Tab* tab, Widget* content, Tab* inFrontOf)
  {
    if (!tab || !content)
      return;

    tab->setTabPane(this);
    Tab* oldSelectedTab = this->getSelectedTab();
    {
      auto iterator = this->tabs.end();
      if (inFrontOf != nullptr)
        iterator = std::find_if(this->tabs.begin(), this->tabs.end(),
                                [inFrontOf](const TabEntry& item) { return item.tab == inFrontOf; });
      this->tabs.insert(iterator, TabEntry(tab, content));
      this->selectedTab = this->tabs.end();
    }
    this->tabContainer.add(tab);
    if (inFrontOf != nullptr)
      this->tabContainer.moveWidgetBefore(tab, inFrontOf);
    if (this->tabs.size() == 1 && oldSelectedTab == nullptr)
      this->setSelectedTab(tab);
    else
      this->setSelectedTab(oldSelectedTab);
    this->triggerResize();
  }

  void TabbedPane::addTab(const std::string& string, Widget* content, Tab* inFrontOf)
  {
    this->addTab(&agui::tab(string), content, inFrontOf);
  }

  void TabbedPane::setSelectedTab(Tab* tab)
  {
    if (tab == nullptr)
    {
      this->setSelectedTab(-1);
      return;
    }

    if (this->getSelectedTab() == tab)
      return;

    int foundIndex = this->getIndex(tab);
    if (foundIndex != -1)
      this->setSelectedTab(foundIndex);
  }

  void TabbedPane::setSelectedTab(int index)
  {
    // set to no selected tab
    if (index == -1)
    {
      if (Tab* selectedTab = this->getSelectedTab())
      {
        selectedTab->lostSelection();
        if (Widget* tabContents = this->getSelectedTabContents())
          this->contentFrame.remove(tabContents, KeepWidgetAlive::True);
      }
      this->selectedTab = this->tabs.end();
      return;
    }
    // set to desired tab
    else if (index < (int)tabs.size())
    {
      // no need to select the same thing
      if (this->tabs[index].tab == this->getSelectedTab())
        return;

      if (Tab* tab = this->getSelectedTab())
        tab->lostSelection();
      Widget* oldSelectedContent = this->getSelectedTabContents();

      this->selectedTab = this->tabs.begin() + index;
      if (Tab* tab = this->getSelectedTab())
        tab->gainedSelection();

      if (Widget* selectedTabContents = this->getSelectedTabContents())
      {
        this->contentFrame.add(selectedTabContents);
        if (this->getFocusedWidget() == nullptr) // Don't steal focus from other widgets just because a tab was added to some tabbed pane
          selectedTabContents->focus();
      }

      if (oldSelectedContent)
        this->contentFrame.remove(oldSelectedContent, KeepWidgetAlive::True);
    }
  }

  void TabbedPane::editSelectedTab(Tab* tab)
  {
    this->setSelectedTab(tab);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::SelectedTabChanged))
      listener.onSelectedTabChange();
  }

  void TabbedPane::editSelectedTab(int index)
  {
    this->setSelectedTab(index);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::SelectedTabChanged))
      listener.onSelectedTabChange();
  }

  Tab* TabbedPane::getSelectedTab() const
  {
    if (this->selectedTab == this->tabs.end())
      return nullptr;
    return *this->selectedTab->tab;
  }

  Tab* TabbedPane::getTab(int index)
  {
    if (index < 0 || index >= int(this->tabs.size()))
      return nullptr;
    return *this->tabs[index].tab;
  }

  bool TabbedPane::isTabFirst(const Tab* tab) const
  {
    if (this->tabs.empty())
      return false;
    return this->tabs.front().tab == tab;
  }

  bool TabbedPane::isTabLast(const Tab* tab) const
  {
    if (this->tabs.empty())
      return false;
    return this->tabs.back().tab == tab;
  }

  agui::Widget* TabbedPane::getSelectedTabContents() const
  {
    if (this->selectedTab == this->tabs.end())
      return nullptr;
    return *this->selectedTab->content;
  }

  bool TabbedPane::mouseDown(const MouseEvent& mouseEvent)
  {
    super::mouseDown(mouseEvent);
    return this->tabContainer.mouseDown(mouseEvent);
  }

  bool TabbedPane::mouseUp(const MouseEvent& mouseEvent)
  {
    super::mouseUp(mouseEvent);
    return this->tabContainer.mouseUp(mouseEvent);
  }

  bool TabbedPane::mouseDrag(const MouseEvent& mouseEvent)
  {
    super::mouseDrag(mouseEvent);
    return this->tabContainer.mouseDrag(mouseEvent);
  }

  void TabbedPane::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    super::setSize(width, height, setSizeInfo);
    int biggestWidth = 0;
    if (this->tabContainer.isVisible())
      this->contentFrame.setSize(this->getContentWidth(), this->getContentHeight() - this->tabContainer.getHeight(), setSizeInfo);
    else
      this->contentFrame.setSize(this->getContentWidth(), this->getContentHeight(), setSizeInfo);
    for (auto& tab : this->tabs)
    {
      if (tab.content->getSize().height > this->contentFrame.getContentHeight() &&
          tab.content->isVerticallySquashable())
        tab.content->setSizeForce(tab.content->getWidth(), this->contentFrame.getContentHeight() - tab.content->getVerticalMargins(), SetSizeInfo(Change::Nothing, Change::Squashing));

      if (tab.content->getSize().width > this->contentFrame.getContentWidth())
      {
        int squashableWidth = tab.content->maximumHorizontalSquashSize();
        if (squashableWidth > 0)
        {
          int toSquash = std::min(squashableWidth, tab.content->getSize().width - this->contentFrame.getContentWidth());
          tab.content->setSizeForce(tab.content->getWidth() - toSquash, this->contentFrame.getContentHeight(), SetSizeInfo(Change::Squashing, Change::Nothing));
        }
      }
      biggestWidth = std::max(biggestWidth, tab.content->getWidth() + tab.content->getHorizontalMargins());
    }
    if (biggestWidth > width - this->contentFrame.getHorizontalPaddings())
      this->setSize(biggestWidth + this->contentFrame.getHorizontalPaddings(), height);
  }

  void TabbedPane::removeTab(Tab* tab)
  {
    int index = getIndex(tab);

    if (index != -1)
      this->removeTab(index);
  }

  void TabbedPane::removeTab(int index)
  {
    if (index < 0 || index >= (int)this->tabs.size())
      return;

    int newSelectedIndex = this->getSelectedIndex();
    if (index <= newSelectedIndex)
      newSelectedIndex--;
    Tab* tab = *this->tabs[index].tab;
    if (tab)
    {
      tab->setTabPane(nullptr);
      this->tabContainer.remove(tab);
    }
    if (Widget* tabContents = *this->tabs[index].content)
    {
      // We need to call both here to cover all cases:
      //   No auto-destruct + no parent
      //   No auto-destruct + parent
      //   Auto-destruct + no parent
      //   Auto-destruct + parent
      tabContents->removeFromParent();
      tabContents->destroyIfautoDestructWhenRemovedFromParent();
    }
    this->tabs.erase(this->tabs.begin() + index);
    this->selectedTab = this->tabs.end();
    this->setSelectedTab(newSelectedIndex);
  }

  int TabbedPane::getIndex(Tab* tab) const
  {
    for (uint32_t i = 0; i < this->tabs.size(); ++i)
      if (this->tabs[i].tab == tab)
        return i;
    return -1;
  }

  int TabbedPane::getSelectedIndex() const
  {
    return this->getIndex(this->getSelectedTab());
  }

  bool TabbedPane::keyDown(const KeyEvent& keyEvent)
  {
    if (keyEvent.getExtendedKey() == EXT_KEY_LEFT)
    {
      if (this->getSelectedIndex() > 0)
        this->setSelectedTab(this->getSelectedIndex() - 1);
      return true;
    }
    if (keyEvent.getExtendedKey() == EXT_KEY_RIGHT)
    {
      this->setSelectedTab(this->getSelectedIndex() + 1);
      return true;
    }
    return false;
  }

  void TabbedPane::focusGained(TabbedIn tabbedIn)
  {
    super::focusGained(tabbedIn);
    if (this->getSelectedTab() != nullptr)
      this->getSelectedTab()->focus();
  }

  void TabbedPane::recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition)
  {
    // We need to draw stuff in special order so that we have some stuff in the tab container - active tab
    // (and frame elements in case of a frame tabbed pane (character gui)) overlapping the top of the content frame
    // while the shadows of that stuff do not. So the order of drawing is:
    // 0) background graphical set
    // 1) inactive tabs - shadows
    // 2) inactive tabs - rest (so that they cover the sides of the shadows)
    // 3) glows - they should not overlap the active tab or sides
    // 4) rest of tab container shadows
    // 5) content frame - complete
    // 6) rest of tab container without shadows

    if (const ElementImageSet* background = this->tabContainer.style.getBackgroundGraphicalSet())
    {
      graphicsContext->setOffset(absolutePosition);
      graphicsContext->pushClippingRect(this, this->getSizeRectangle(), true);
      background->base.draw(PaintEvent(enabled, graphicsContext), this->getSizeRectangle(), absolutePosition);
      graphicsContext->popClippingRect();
    }

    std::vector<Widget*> toDrawLater; // active tab + frame tabs stuff

    const int leftPadding = this->getLeftPadding();
    const int topPadding = this->getTopPadding();
    const int tabsLeftPadding = leftPadding + this->tabContainer.getLeftPadding();
    const int tabsTopPadding = topPadding + this->tabContainer.getTopPadding();

    if (this->tabContainer.isVisible())
    {
      // 1) inactive tabs - shadows, store the rest
      for (Widget* widget : this->tabContainer)
        if (Tab* tab = dynamic_cast<Tab*>(widget))
          if (tab != this->getSelectedTab())
            widget->recursivePaintShadows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), tabsLeftPadding, tabsTopPadding), true);
          else
            toDrawLater.push_back(widget);
        else
          toDrawLater.push_back(widget);

      // 2) inactive tabs - rest
      for (Widget* widget : this->tabContainer)
        if (Tab* tab = dynamic_cast<Tab*>(widget))
          if (tab != this->getSelectedTab())
          {
            const agui::Point rootAbsolutePosition = Point(absolutePosition, widget->getLocation(), tabsLeftPadding, tabsTopPadding);
            graphicsContext->setOffset(rootAbsolutePosition);
            widget->recursivePaintChildren(enabled, graphicsContext, rootAbsolutePosition);
          }
    }
    graphicsContext->setOffset(this->getAbsolutePosition());

    // 2) glows
    this->recursivePaintGlows(enabled, graphicsContext, absolutePosition, true);

    // 3) rest of tab container shadows
    for (Widget* widget : toDrawLater)
      widget->recursivePaintShadows(enabled, graphicsContext, Point(absolutePosition, widget->getLocation(), tabsLeftPadding, tabsTopPadding), true);

    // 4) content frame
    {
      const agui::Point rootAbsolutePosition = Point(absolutePosition, this->contentFrame.getLocation(), leftPadding, topPadding);
      graphicsContext->setOffset(rootAbsolutePosition);
      this->contentFrame.recursivePaintChildren(enabled, graphicsContext, rootAbsolutePosition);
    }
    graphicsContext->setOffset(this->getAbsolutePosition());

    // 5) rest of tab container without shadows, but draw shadows of their children
    bool originalPaintShadows = graphicsContext->shadowView;
    for (Widget* widget : toDrawLater)
    {
      graphicsContext->shadowView = false;
      const agui::Point rootAbsolutePosition = Point(absolutePosition, widget->getLocation(), tabsLeftPadding, tabsTopPadding);
      graphicsContext->setOffset(rootAbsolutePosition);
      widget->recursivePaintChildren(enabled, graphicsContext, rootAbsolutePosition);
      graphicsContext->shadowView = originalPaintShadows;
      widget->recursivePaintShadows(enabled, graphicsContext, rootAbsolutePosition, true, false); // shadows of children
    }
  }

  void TabbedPane::recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    if (forceInFrame)
      super::recursivePaintGlows(enabled, graphicsContext, absolutePosition, forceInFrame, includeThis);
  }

  void TabbedPane::recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame, bool includeThis)
  {
    (void)enabled;
    (void)graphicsContext;
    (void)absolutePosition;
    (void)forceInFrame;
    (void)includeThis;
  }

  TabbedPane::TabEntry::TabEntry(Tab* tab, Widget* content)
    : tab(tab)
    , content(content)
  {}

  void TabbedPane::flagAllChildrenForDestruction()
  {
    super::flagAllChildrenForDestruction();
    for (size_t i = 0; i < tabs.size(); ++i)
    {
      this->tabs[i].tab->flagForDestruction();
      this->tabs[i].tab->flagAllChildrenForDestruction();

      this->tabs[i].content->flagForDestruction();
      this->tabs[i].content->flagAllChildrenForDestruction();
    }
  }

  void TabbedPane::resizeToContentsRecursive()
  {
    for (int i = 0; i < (int)tabs.size(); ++i)
    {
      this->tabs[i].content->resizeToContentsRecursive();
      this->tabs[i].tab->resizeToContents();
    }
    this->tabContainer.resizeToContentsRecursive();
    this->resizeToContents();
  }

  void TabbedPane::resizeToContents()
  {
    this->tabContainer.resizeToContents();
    this->adjustWidgetContainer();
    if (this->tabContainer.isVisible())
    {
      this->setContentSize(std::max(this->getWidestContentTab() + this->contentFrame.getHorizontalPaddings(), this->tabContainer.getWidth()),
                                    this->getHighestContentTab() + this->contentFrame.getVerticalPaddings() + this->tabContainer.getHeight());
      if (this->tabContainer.style.isHorizontallyStretchable() == agui::StretchRule::On ||
          this->tabContainer.style.isHorizontallyStretchable() == agui::StretchRule::StretchAndExpand)
        this->tabContainer.setSize(std::max(this->tabContainer.getWidth(), this->getContentWidth()), this->tabContainer.getHeight());
    }
    else
      this->setContentSize(this->getWidestContentTab() + this->contentFrame.getHorizontalPaddings(),
                           this->getHighestContentTab() + this->contentFrame.getVerticalPaddings());
  }

  void TabbedPane::displaySizeChanged()
  {
    super::displaySizeChanged();

    const Tab* selectedTab = this->getSelectedTab();
    for (auto& item : this->tabs)
      // Selected tab will get touched through the private children sweep
      if (item.tab != selectedTab)
        // item.first (the tab) will get touched through the private children sweep as well
        item.content->displaySizeChangedRecursive();
  }

  void TabbedPane::postDisplaySizeChanged()
  {
    super::postDisplaySizeChanged();

    const agui::Tab* selectedTab = this->getSelectedTab();
    for (auto& item : this->tabs)
      // Selected tab will get touched through the private children sweep
      if (item.tab != selectedTab)
        // item.first (the tab) will get touched through the private children sweep as well
        item.content->postDisplaySizeChanged();
  }

  int TabbedPane::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;
    int maxHeight = 0;
    for (auto& item : this->tabs)
      if (!this->resizeToCurrentTabContents || item.tab == this->getSelectedTab())
        maxHeight = std::max(maxHeight, item.content->getHeight() - item.content->maximumVerticalSquashSize());
    return std::max(0, this->getContentHeight() - (this->tabContainer.getHeight() + maxHeight));
  }

  int TabbedPane::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;
    int maxWidth = 0;
    for (auto& item : this->tabs)
      if (!this->resizeToCurrentTabContents || item.tab == this->getSelectedTab())
        maxWidth = std::max(maxWidth, item.content->getWidth() + item.content->getHorizontalMargins() - item.content->maximumHorizontalSquashSize());
    return std::max(0, std::min(this->getContentWidth() - maxWidth - this->contentFrame.getHorizontalPaddings(), this->getContentWidth() - this->tabContainer.getWidth() - this->tabContainer.maximumHorizontalSquashSize()));
  }

  int TabbedPane::getWidestContentTab()
  {
    int widestWidget = 0;
    for (auto& item : this->tabs)
      if (!this->resizeToCurrentTabContents || item.tab == this->getSelectedTab())
        widestWidget = std::max(widestWidget, item.content->getWidth() + item.content->getHorizontalMargins());
    return widestWidget;
  }

  int TabbedPane::getHighestContentTab()
  {
    int highestWidget = 0;
    for (auto& item : this->tabs)
      if (!this->resizeToCurrentTabContents || item.tab == this->getSelectedTab())
        highestWidget = std::max(highestWidget, item.content->getHeight() + item.content->getVerticalMargins());
    return highestWidget;
  }
}

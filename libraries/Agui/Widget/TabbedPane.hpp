#pragma once
#include <Agui/GenericTargeter.hpp>
#include <Agui/Widget.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Table.hpp>
#include <Agui/Widget/TabbedPaneStyle.hpp>
namespace agui
{
  class KeyEvent;
  class Tab;
  class PaintEvent;
}

namespace agui
{
  /** Container to hold Tabs. */
  class TabbedPane final : public Widget
  {
    using super = Widget;
  public:
    TabbedPane(const TabbedPaneStyle* parentStyle = &TabbedPane::defaultStyle);
    virtual ~TabbedPane(void);
    virtual void reapplySubStyles() override;
    virtual void flagAllChildrenForDestruction() override; // Flags, in addition, all Tabs and their associated Widget.
    virtual void focusGained(TabbedIn tabbedIn) override;
    virtual bool keyDown(const KeyEvent& keyEvent) override; // Selects the next Tab and gives it focus.
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override; // Adjusts the size of the TabbedPane, done automatically when it resizes.
    void addTab(Tab& tab, Widget& content) { this->addTab(&tab, &content); }
    /** Adds a Tab and its associated content Widget.
     * @param inFrontOf, the tab will be inserted in front of this one, or at the end when it is nullptr */
    void addTab(Tab* tab, Widget* content, Tab* inFrontOf = nullptr);
    void addTab(const std::string& string, Widget* content, Tab* inFrontOf = nullptr);
    void removeTab(Tab* tab); // Removes a Tab and its associated content Widget.
    void removeTab(int index); // Removes a Tab and its associated content Widget by index.
    int getIndex(Tab* tab) const; // @return The index of the parameter tab, -1 if not found.
    int getSelectedIndex() const; // @return The index of the selected tab, -1 if not found.
    void setSelectedTab(Tab* tab); // Sets the selected Tab to the parameter one and shows its content.
    void setSelectedTab(int index); // Sets the selected Tab to the parameter index one and shows its content.
    void editSelectedTab(Tab* tab); // sets selected tab as it was triggered by user, it fires the selectedTabChanged
    void editSelectedTab(int index); // sets selected tab as it was triggered by user, it fires the selectedTabChanged
    Tab* getSelectedTab() const; // @return The selected Tab or nullptr if none are selected.
    Tab* getTab(int index);
    bool isTabFirst(const Tab* tab) const;
    bool isTabLast(const Tab* tab) const;
    Widget* getSelectedTabContents() const; // @return The selected Tab contents or nullptr if none are selected.
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    virtual void resizeToContentsRecursive() override;
    virtual void resizeToContents() override;
    virtual void displaySizeChanged() override;
    virtual void postDisplaySizeChanged() override;
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;
    int getWidestContentTab();
    int getHighestContentTab();
    bool hasTabs() const { return !this->tabs.empty(); }
    int getTabCount() const { return int(this->tabs.size()); }
    virtual void clear() override;
    void clearTabs();
    virtual Style* getStyle() override { return &this->style; }
    void setResizeToCurrentTabContents(bool value);
    virtual bool genericSearch(const LowercaseString& filter) override;
    void onSelectedTabChange(GenericTargetable* owner, std::function<void()> callback);
  protected:
    void adjustWidgetContainer();
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;
    virtual void recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;

  public:
    static TabbedPaneStyle defaultStyle;

    TabbedPaneStyle style;
    Table tabContainer;
    Frame contentFrame;
  private:
    struct TabEntry
    {
      TabEntry(Tab* tab, Widget* content);
      GenericTargeter<Tab> tab;
      GenericTargeter<Widget> content;
    };
    using TabsType = std::vector<TabEntry>;
    TabsType tabs;
    TabsType::iterator selectedTab;
    bool resizeToCurrentTabContents = false;
  };
}

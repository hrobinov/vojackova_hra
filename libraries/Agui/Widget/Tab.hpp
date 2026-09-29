#pragma once
#include "Agui/Clickable.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/TabStyle.hpp"

namespace agui
{
  class TabbedPane;
  /** Used by TabbedPane */
  class Tab : public Clickable
  {
    using super = Clickable;
  public:
    Tab(const TabStyle* parentStyle = nullptr);
    Tab(const std::string& text, const TabStyle* parentStyle = nullptr);
    virtual ~Tab();
    virtual Style* getStyle() override { return &this->style; }
    /** Uses left and right arrow keys to navigate tabs in the tab pane.
     * If the TabbedPane this tab belongs to gets focus, it will be forwarded to the selected tab. */
    virtual bool keyDown(const KeyEvent& keyEvent) override;
    /** Uses left and right arrow keys to navigate tabs in the tab pane.
     * If the TabbedPane this tab belongs to gets focus, it will be forwarded  to the selected tab. */
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;
    /** Sets the TabbedPane this Tab belongs to. Called by the TabPane to add itself to this Tab. */
    virtual void setTabPane(TabbedPane* pane);
    virtual void gainedSelection(); // Called by the TabbedPane when this Tab becomes the selected Tab.
    virtual void lostSelection(); // Called by the TabbedPane when this Tab is no longer the selected Tab.
    virtual bool mouseDown(const MouseEvent& mouseEvent) override; // Sets this tab as the selected tab (if possible).
    virtual bool isSelectedTab() const; // @return True if this tab is the selected tab in the TabbedPane it belongs to.
    virtual bool shouldPlaySound() const override { return !this->isSelectedTab(); }
    virtual void resizeToContents() override; // Resizes the Tab to fit its caption.
    virtual void setText(const std::string& text) override;
    virtual void setText(std::string&& text) override;
    void setBadgeText(const std::string& text);
    void setBadgeText(std::string&& text);
    const std::string& getBadgeText() const { return this->badgeText; }

  protected:
    virtual void paintComponent(const PaintEvent& paintEvent, const Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent& paintEvent, const Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const Point& absolutePosition) override;
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const Point& absolutePosition) override;
  private:
    const ElementImageSet* getCurrentImageSet() const;
    const ElementImageSet* getCurrentBadgeImageSet() const;
    Color getCurrentFontColor() const;
    Color getCurrentBadgeFontColor() const;
    Rectangle getCurrentDrawingSize() const;

  public:
    static TabStyle defaultStyle;

    TabStyle style;
  private:
    TabbedPane* tabPane = nullptr;
    std::string badgeText;
  };
}

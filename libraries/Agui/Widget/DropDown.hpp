#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/DropDownStyle.hpp"
#include "Agui/Widget/ListBox.hpp"

namespace agui
{
  class DropDown : public Clickable
  {
    using super = Widget;
  public:
    DropDown(const DropDownStyle* parentStyle = &DropDown::defaultStyle);
  protected:
    virtual void setupListBox(); // Prepares the internal ListBox to be used.
    virtual void positionListBox(); // Positions the internal ListBox when it is shown.
    virtual void showDropDown(); // Shows the internal ListBox.
    virtual void hideDropDown(); // Hides the internal ListBox.
    virtual void drawText(const PaintEvent& paintEvent);
    virtual void paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void handleKeyboard(const KeyEvent& keyEvent);
    void paintBackgroundLayer(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layer);
  public:
    void hideDropdownWithoutFocus();
    virtual void reapplySubStyles() override;
    virtual Style* getStyle() override { return &this->style; }
    virtual void addItem(const std::string& item) { this->addItem(std::string(item)); }
    virtual void addItem(std::string&& item);
    virtual void addItems(std::vector<std::string>&& items);
    virtual void addItemAt(const std::string& item, int index) { this->addItemAt(std::string(item), index); }
    virtual void addItemAt(std::string&& item, int index);
    virtual void setItemAt(const std::string& item, int index) { this->setItemAt(std::string(item), index); }
    virtual void setItemAt(std::string&& item, int index);
    virtual void clearItems();
    virtual void setItemToolTip(int index, std::string&& text);
    virtual std::string getItemAt(int index) const;
    /** @return The index of the first found instance of the parameter string in the internal ListBox, or -1 if not found.  */
    virtual int getIndexOf(const std::string& item) const;
    virtual void removeItem(const std::string& item);
    virtual void removeItemAt(int index); // Removes an item from the internal ListBox at the specified index.
    const std::vector<ListBoxItem>& getItems() const;
    virtual bool isDropDownShowing() const; // @return if the internal ListBox is visible.
    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;
    virtual void setSelectedIndex(int index); // Sets the selected index. The caption will change to accommodate.
    void editSelectedIndex(int index);
    virtual void setSelectedItem(const std::string& item);
    virtual int getSelectedIndex() const;
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual void onSizeChanged(Dimension originalSize) override;
    virtual void setLocation(const Point& location) override;
    virtual void setLocation(int width, int height) override;
    /** Sets the maximum height of the internal ListBox. If the ListBox overflows its vertical scroll bar will appear. */
    void setListPositionOffset(const Point& offset); // Sets the offset for the position of the ListBox when it is shown.
    const Point& getListPositionOffset() const; // @return The offset for the position of the ListBox when it is shown.
    void setListSizePadding(const Dimension& padding); // Sets the offset for the position of the ListBox when it is shown.
    const Dimension& getListSizePadding() const; // @return The offset for the position of the ListBox when it is shown.
    virtual bool mouseEnter(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual bool isMouseInside() const;
    virtual void resizeToContents() override;
    virtual ToolTip* createToolTip() override;
    int getItemCount() const;
    virtual const ElementImageSet* getBorderImageSet() const override;
    void reserveSpaceFor(const std::string& text);
    void updateSelectedIndex();
    const ElementImageSet* getCurrentImageSet() const;
    const Color& getCurrentFontColor() const;
    bool dispatchOnBeforeDropdownIsShown();
    virtual Widget* getGameControllerHoveredChildInternal() override { return this; }
    void onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback);
    void onBeforeDropdownIsShown(GenericTargetable* owner, std::function<bool()> callback);
    agui::Widget* getListBoxAt(uint32_t index);
  private:
    bool processKey(const KeyEvent& keyEvent);
  public:

    static DropDownStyle defaultStyle;

    DropDownStyle style;
  private:
    Point listPosOffset;
    Dimension listSizeIncrease;
    int selectedIndex = -1;
    ListBox listBox;
    bool mouseInside = false;
  };
}

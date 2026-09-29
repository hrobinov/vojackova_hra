#pragma once
#include "Agui/KeyboardInputBlockType.hpp"
#include "Agui/ScrollPolicy.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/ListBoxStyle.hpp"
#include "Agui/Widget/VerticalScrollPane.hpp"
#include <memory>
namespace agui { class KeyboardInputBlockerBase; }

namespace agui
{
  /** Supports three types of selections, Single selection, Multi selection and Extended multi selection */
  /** In theory, that is. Currently, only single selection works. Some code exists for the others. */
  struct ListBoxItem
  {
    ListBoxItem(ListBox* parent, TextButton* button);
    ListBoxItem(ListBox* parent, std::string&& text, const ButtonStyle* defaultStyle);
    bool isSelected() const { return this->button->isToggled(); }
    void setSelected(bool selected);

    std::unique_ptr<TextButton> button;
    void* tag = nullptr;
  };

  class ListBox : public Widget
  {
    using super = Widget;
  public:
    ListBox(const ListBoxStyle* parentStyle = &ListBox::defaultStyle);
    virtual ~ListBox();

  private:
    void clearKeyboardInputBlocker();
    void createKeyboardInputBlocker();
  protected:
    void keyAction(ExtendedKeyEnum key, bool shift); // Handles keyboard actions like arrow keys.

    virtual void setDragIndex(int index);
    virtual void dragItemAt(int fromIndex, int toIndex);
    virtual void setKeyboardInputBlockType(KeyboardInputBlockType type);

  public:
    enum class MoveDirection {Up, Down};
    virtual void makeSelection(int selection, MoveDirection moveDirection); // Used internally to make a selection.
    virtual Style* getStyle() override { return &this->style; }
    virtual int getIndexAtPoint(const Point& p) const; // The zero based index of the item at this point.
    virtual int getIndexOfWidget(const Widget* widget) const;
    void moveToSelection(int selection, ScrollMode scrollMode); // Scrolls / moves to the parameter index.
    void setHandleMouseWheel(bool value) { this->handleMouseWheel = value; }
    virtual void focusGained(TabbedIn tabbedIn) override;
    virtual void focusLost() override;

    virtual void reapplySubStyles() override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    void itemMouseDown(const MouseEvent& mouseEvent);
    void itemMouseUp();
    void itemMouseDoubleClick(const MouseEvent& mouseEvent);
    void itemToggle(const Widget* source, bool leftClick = true);
    void itemMouseDrag(const MouseEvent& mouseEvent);
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;

    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;
    /** @return True if the ListBox will wrap to the first or last item when the bottom or top is reached.
     * Does not work with Multiselect, and works with MultiselectExtended if only one item is selected. */
    virtual bool isWrapping() const;
    /** Sets whether or not the ListBox will wrap to the first or last item when the bottom or top is reached.
     * Does not work with Multiselect, and works with MultiselectExtended if only one item is selected. */
    virtual void setWrapping(bool wrapping);
    virtual int getBottomSelectedIndex() const;
    virtual void selectRange(int startIndex, int endIndex); // Selects a range of items. Each item will raise a selection event.
    virtual bool indexExists(int index) const;
    void addItem(const std::string& item) { this->addItem(std::string(item)); }
    virtual void addItem(TextButton* button);
    virtual void addItem(std::string&& item);
    virtual void addItemWithRemark(std::string&& item, std::string&& remark);
    virtual void addItems(const std::string& items); // Adds multiple items by parsing newline characters.
    virtual void addItems(const std::vector<std::string>& items);
    virtual void addItems(std::vector<std::string>&& items);
    virtual void removeItem(const std::string& item); // Removes the first instance of this item.
    virtual void removeItemAt(int index);
    virtual bool consumesKeyWhenFocused(KeyEnum key) const override;
    void addItemAt(const std::string& item, int index) { this->addItemAt(std::string(item), index); }
    virtual void addItemAt(std::string&& item, int index);
    /** Takes ownership of the given strings. If an item is updated its style is reset and any references to that item are invalidatedaddItems. */
    virtual void updateItems(std::vector<std::string>&& items);
    virtual int getItemCount() const;
    /** The index of the first found instance of the parameter string or -1 if not found. */
    virtual int getIndexOf(const std::string& item) const;
    /** The string of the first found instance of the parameter string or "" if not found. */
    virtual std::string getItemAt(int index) const;
    TextButton* getButtonAt(int index) const;
    virtual void setItemAt(int index, const std::string& value);
    virtual int getSelectedIndex() const; // The topmost selected index or -1 if nothing is selected.
    /** @return the topmost selected index or an empty string if nothing is selected. */
    virtual std::string getSelectedItem() const;
    virtual void setSelectedIndex(int index); // Sets the selected index. This will be the only selected index.
    /** Called when the index has to be changed because of user action.
     * It will call the dispatchUserChangedSelection method as well as userChangedSelection */
    void editSelectedIndex(int index);
    virtual void clearItems();  // Erases and removes all items. Sends a selection event of -1.
    virtual void clearSelectedIndexes(int avoid);
    virtual void setSingleSelection(); // Sets that only one item at a time may be selected (Default).
    virtual bool isSingleSelection() const; // If only one item at a time may be selected (Default).
    virtual void setNewItemColor(const agui::Color& color); // Sets the text color for a newly added item.
    virtual const agui::Color& getNewItemColor() const; // The text color for a newly added item.
    const ListBoxItem& getListItemAt(int index) const;
    void setItemTextColor(const agui::Color& color, int index);
    void setItemStrikethrough(bool value, int index);
    void setItemStyle(int index, const ButtonStyle* itemStyle); // When itemStyle is nullptr will set the default item style.
    void setItemEnabled(int index, bool enabled);
    void setItemVisible(int index, bool visible);
    void setItemToolTip(int index, const std::string& text);
    void setItemToolTip(int index, std::string&& text);
    void setItemToolTipCreator(int index, agui::ToolTipCreatorBase* toolTipCreator);
    void setItemRemark(int index, const std::string& text);
    bool isItemEnabled(int index) const;
    bool isItemVisible(int index) const;
    const std::vector<ListBoxItem>& getItems() const { return this->items; }
    virtual bool isListBox() const override { return true; }
    bool getSelectByTyping() const;
    void setSelectByTyping(bool value);
    void selectBestSelectedItemByBeginingText(const std::string& text);
    void setSelectedItem(const std::string& item);
    void setSelectNextOnEraseOfSelected(bool value);
    bool getSelectNextOnEraseOfSelected() const;
    void selectLastEnabledItem();
    void setEnableDrag(bool value);
    bool getEnableDrag() const;
    int getVScrollPosition() const;
    void setVScrollPosition(int position);
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    virtual void resizeToContents() override;
    int requiredWidth() const;
    virtual bool genericSearch(const LowercaseString& filter) override;
    virtual void reselectAfterSearch(const LowercaseString& filter);
    void onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback);
    void onItemSelect(GenericTargetable* owner, std::function<void(int index, bool leftClick)> callback);
    void onItemDoubleClick(GenericTargetable* owner, std::function<void(int index)> callback);

    friend struct ListBoxItem;
    static ListBoxStyle defaultStyle;

    ListBoxStyle style;

  private:
    int dragIndex = -1;
    bool wrapping = false;
    bool enableDrag = false;
    bool handleMouseWheel = true;
    bool selectNextOnEraseOfSelected = false;
  public:
    bool selectAsReactionToSearch = true;
    VerticalScrollPane itemHolder;
  private:
    std::vector<ListBoxItem> items;

    agui::Color newItemColor;

    // Select by typing properties
    bool selectByTyping = false;
    double lastKeyTimestamp = 0;
    std::string searchText;

    KeyboardInputBlockerBase* keyboardInputBlocker = nullptr;
    KeyboardInputBlockType keyboardBlockType = KeyboardInputBlockType::None;
  };
}

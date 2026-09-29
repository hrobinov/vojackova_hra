#include "Agui/ElementImageSet.hpp"
#include "Agui/Exception.hpp"
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/ListBox.hpp"
#include "Agui/Widget/ScrollBar.hpp"
#include <Agui/EventDispatchHelper.hpp>
#include <Agui/KeyboardInputBlockType.hpp>
#include <Agui/KeyboardInputBlockerFactory.hpp>
#include <Agui/UTF8.hpp>
#include <Agui/UTF8.hpp>
#include <Agui/Util.hpp>
#include <algorithm>
#include <cassert>
#include <memory>

namespace agui
{
  ListBoxStyle ListBox::defaultStyle;

  ListBoxItem::ListBoxItem(ListBox* parent, TextButton* button)
    : button(button)
  {
    this->button->stretchHorizontally();
    this->button->squashHorizontally();
    parent->itemHolder.add(this->button.get());
    this->button->onMouseDown(parent, [parent](const MouseEvent& mouseEvent) { parent->itemMouseDown(mouseEvent); });
    this->button->onMouseUp(parent, [parent](const MouseEvent&) { parent->itemMouseUp(); });
    this->button->onDoubleClick(parent, [parent](const MouseEvent& mouseEvent) { parent->itemMouseDoubleClick(mouseEvent); });
    this->button->onToggle(parent, [parent, button = this->button.get()](bool leftClick) { parent->itemToggle(button, leftClick); });
    this->button->onMouseDrag(parent, [parent](const MouseEvent& mouseEvent) { parent->itemMouseDrag(mouseEvent); });
    this->button->setToggleButton(true);
    this->button->setAutoUntoggle(false);
    this->button->setFocusParentWhenNotFocusable(true);
  }

  ListBoxItem::ListBoxItem(ListBox* parent, std::string&& text, const ButtonStyle* defaultStyle)
    : ListBoxItem(parent, new TextButton(std::move(text), defaultStyle))
  {}

  void ListBoxItem::setSelected(bool selected)
  {
    this->button->setToggleState(selected);
  }

  ListBox::ListBox(const ListBoxStyle* defaultStyle)
    : style(this, defaultStyle)
    , itemHolder(this->style.getScrollPaneStyle())
  {
    *this << this->itemHolder;
    this->itemHolder.contentFlow.setFocusParentWhenNotFocusable(true);
    this->itemHolder.setFocusParentWhenNotFocusable(true);
    this->setFocusable(true);
    this->keepFocusWhenClickingOutsideOnNonFocusableWidget();
  }

  ListBox::~ListBox()
  {
    this->clearKeyboardInputBlocker();
  }

  void ListBox::addItem(std::string&& item)
  {
    this->addItemAt(std::move(item), this->getItemCount());
  }

  void ListBox::addItemWithRemark(std::string&& item, std::string&& remark)
  {
    const int childIndex = this->getItemCount();
    this->addItemAt(std::move(item), childIndex);
    this->setItemRemark(childIndex, std::move(remark));
  }

  void ListBox::addItem(TextButton* button)
  {
    this->items.push_back(ListBoxItem(this, button));
    this->setDragIndex(-1);
    this->triggerResize();
  }

  void ListBox::removeItem(const std::string& item)
  {
    // remove first occurrence of item
    for (auto it = this->items.begin(); it != this->items.end(); ++it)
      if (it->button->getText() == item)
      {
        this->items.erase(it);
        this->triggerResize();
        return;
      }
  }

  int ListBox::getItemCount() const
  {
    return int(items.size());
  }

  void ListBox::addItemAt(std::string&& item, int index)
  {
    if (this->indexExists(index) || index == this->getItemCount())
    {
      this->items.insert(this->items.begin() + index,
                         ListBoxItem(this,
                                     std::move(item),
                                     this->style.getItemStyle()));
      if (index + 1 < this->getItemCount())
        this->itemHolder.contentFlow.moveWidgetBefore(this->items[index].button.get(), this->items[index + 1].button.get());
      this->setDragIndex(-1);
      this->triggerResize();
    }
  }

  void ListBox::updateItems(std::vector<std::string>&& newItems)
  {
    bool needsUpdate = this->items.size() != newItems.size();
    if (!needsUpdate)
      for (uint32_t i = 0; i < this->items.size(); ++i)
        if (this->items[i].button->getText() != newItems[i])
        {
          needsUpdate = true;
          break;
        }

    if (needsUpdate)
    {
      this->clearItems();
      this->addItems(std::move(newItems));
    }
  }

  bool ListBox::indexExists(int index) const
  {
    return index >= 0 && index < (int)this->items.size();
  }

  void ListBox::removeItemAt(int index)
  {
    if (this->indexExists(index))
    {
      this->items.erase(this->items.begin() + index);

      this->setDragIndex(-1);
      this->triggerResize();
    }
  }

  int ListBox::getIndexOf(const std::string& item) const
  {
    for (int i = 0; i < int(this->items.size()); ++i)
      if (this->items[i].button->getText() == item)
        return i;
    return -1;
  }

  int ListBox::getSelectedIndex() const
  {
    for (int i = 0; i < int(this->items.size()); ++i)
      if (this->items[i].isSelected())
        return i;
    return -1;
  }

  std::string ListBox::getSelectedItem() const
  {
    for (auto& item : this->items)
      if (item.isSelected())
        return item.button->getText();
    return std::string();
  }

  void ListBox::setSelectedIndex(int index)
  {
    if (this->indexExists(index) || index == -1)
    {
      if (index == this->getSelectedIndex() &&
          this->getSelectedIndex() == this->getBottomSelectedIndex())
        return;

      this->clearSelectedIndexes(index);
      if (index >= 0)
        this->items[index].setSelected(true);
    }
  }

  void ListBox::clearItems()
  {
    this->items.clear();
  }

  void ListBox::clearSelectedIndexes(int avoid)
  {
    for (int i = 0; i < int(this->items.size()); ++i)
      if (i != avoid)
        this->items[i].setSelected(false);
  }

  void ListBox::keyAction(ExtendedKeyEnum key, bool)
  {
    GenericTargeter<ListBox> self(this);
    switch (key)
    {
      case EXT_KEY_UP:
      {
        int selectedIndex = this->getSelectedIndex();
        if (this->isWrapping() && selectedIndex == 0)
          this->makeSelection(this->getItemCount() - 1, MoveDirection::Up);
        else if (selectedIndex != -1)
          this->makeSelection(selectedIndex - 1, MoveDirection::Up);
        else if (this->getItemCount() > 0)
          this->makeSelection(this->getItemCount() - 1, MoveDirection::Up);
        if (self)
          this->moveToSelection(this->getSelectedIndex(), ScrollMode::InView);
        break;
      }
      case EXT_KEY_DOWN:
      {
        int selectedIndex = this->getSelectedIndex();
        if (this->isWrapping() && selectedIndex == this->getItemCount() - 1)
          this->makeSelection(0, MoveDirection::Down);
        else if (selectedIndex != -1 || this->getItemCount() > 0)
          this->makeSelection(selectedIndex + 1, MoveDirection::Down);
        if (self)
          this->moveToSelection(this->getSelectedIndex(), ScrollMode::InView);
        break;
      }
      case EXT_KEY_PAGE_DOWN: this->itemHolder.scrollToV(this->itemHolder.verticalScrollBar.getValue() + this->itemHolder.verticalScrollBar.getLargeAmount()); break;
      case EXT_KEY_PAGE_UP: this->itemHolder.scrollToV(this->itemHolder.verticalScrollBar.getValue() - this->itemHolder.verticalScrollBar.getLargeAmount()); break;
      case EXT_KEY_HOME:
        this->makeSelection(0, MoveDirection::Down);
        if (self)
          this->moveToSelection(this->getSelectedIndex(), ScrollMode::InView);
        break;
      case EXT_KEY_END:
        this->makeSelection(this->getItemCount() - 1, MoveDirection::Up);
        if (self)
          this->moveToSelection(this->getSelectedIndex(), ScrollMode::InView);
        break;
      default: break;
    }
  }

  void ListBox::setDragIndex(int index)
  {
    this->dragIndex = index;
  }

  bool ListBox::keyDown(const KeyEvent& keyEvent)
  {
    static_assert(KEY_0 < KEY_9, "Key numbers aren't in sequential order");
    static_assert(KEY_A < KEY_Z, "Key letters aren't in sequential order");

    this->keyAction(keyEvent.getExtendedKey(), keyEvent.shift());
    if (keyEvent.getKey() == KEY_SPACE || keyEvent.getKey() == KEY_ENTER)
      this->dispatchConfirm(keyEvent);
    if (this->selectByTyping &&
        (keyEvent.getKey() >= KEY_0 &&
         keyEvent.getKey() <= KEY_9 ||
         keyEvent.getKey() >= KEY_A &&
         keyEvent.getKey() <= KEY_Z ||
         keyEvent.getKey() == KEY_UNDERSCORE ||
         keyEvent.getKey() == KEY_SPACE ||
         keyEvent.getKey() == KEY_HYPHEN ||
         keyEvent.getKey() == KEY_PERIOD))
    {
      std::string keyString = keyEvent.getUtf8String();
      if (keyEvent.getTimeStamp() - this->lastKeyTimestamp > 2 /* 2 seconds */)
        this->searchText = std::string();
      this->lastKeyTimestamp = keyEvent.getTimeStamp();
      this->searchText.append(keyString);
      this->selectBestSelectedItemByBeginingText(this->searchText);
    }
    if (this->selectByTyping &&
        (keyEvent.getExtendedKey() == EXT_KEY_UP ||
         keyEvent.getExtendedKey() == EXT_KEY_DOWN))
      this->lastKeyTimestamp = 0;
    return true;
  }

  bool ListBox::keyRepeat(const KeyEvent& keyEvent)
  {
    this->keyAction(keyEvent.getExtendedKey(), keyEvent.shift());
    return true;
  }

  void ListBox::makeSelection(int selection, MoveDirection moveDirection)
  {
    if (!this->indexExists(selection))
      return;

    while (!this->items[selection].button->isEnabled() || !this->items[selection].button->isVisible())
    {
      selection = selection + (moveDirection == MoveDirection::Up ? -1 : 1);
      if (moveDirection == MoveDirection::Up && selection < 0 ||
          moveDirection == MoveDirection::Down && selection >= int(this->items.size()))
        return;
    }
    this->editSelectedIndex(selection);
  }

  void ListBox::dragItemAt(int fromIndex, int toIndex)
  {
    if (fromIndex == toIndex)
      return;
    if (!this->indexExists(fromIndex) || !this->indexExists(toIndex))
      return;

    std::swap(this->items[fromIndex], this->items[toIndex]);
    this->itemHolder.contentFlow.swapChildren(fromIndex, toIndex);

    this->setDragIndex(-1);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnItemDrag))
      listener.itemDrag(fromIndex, toIndex);
  }

  bool ListBox::mouseLeave(const MouseEvent& mouseEvent)
  {
    Widget::mouseLeave(mouseEvent);
    this->setDragIndex(-1);
    return true;
  }

  void ListBox::addItems(const std::string& items)
  {
    if (items.empty())
      return;

    size_t curpos = 0;
    size_t len = 0;

    // parse the string for newlines and add an item when a newline is found
    for (size_t i = 0; i < items.size(); ++i)
    {
      if (items[i] == '\n')
      {
        if (len > 0)
        {
          this->items.emplace_back(this, items.substr(curpos, len), this->style.getItemStyle());
          len = 0;
        }
        curpos = i + 1;
      }
      else
        len++;
    }

    if (curpos < items.size() && len > 0)
      this->items.emplace_back(this, items.substr(curpos, len), this->style.getItemStyle());

    this->setDragIndex(-1);
    this->triggerResize();
  }

  void ListBox::addItems(const std::vector<std::string>& items)
  {
    std::vector<std::string> itemsToAdd = items;
    this->addItems(std::move(itemsToAdd));
  }

  void ListBox::addItems(std::vector<std::string>&& items)
  {
    for (auto& item : items)
      this->items.emplace_back(this, std::move(item), this->style.getItemStyle());
    this->setDragIndex(-1);
    this->triggerResize();
  }

  int ListBox::getIndexAtPoint(const Point& p) const
  {
    int i = 0;
    for (const ListBoxItem& item : this->items)
    {
      if (item.button->intersectionWithPoint(p))
        return i;
      ++i;
    }
    return -1;
  }

  int ListBox::getIndexOfWidget(const Widget* widget) const
  {
    for (size_t index = 0; index < this->items.size(); index++)
      if (this->items[index].button == widget)
        return int(index);
    return -1;
  }

  void ListBox::moveToSelection(int selection, ScrollMode scrollMode)
  {
    if (selection == -1 || !this->indexExists(selection))
      return;
    this->itemHolder.scrollToMakeWidgetVisible(this->items[selection].button.get(), scrollMode);
    this->setDragIndex(-1);
  }

  std::string ListBox::getItemAt(int index) const
  {
    if (!this->indexExists(index))
      return std::string();
    return items[index].button->getText();
  }

  TextButton* ListBox::getButtonAt(int index) const
  {
    if (!this->indexExists(index))
      return nullptr;
    else
      return this->items[index].button.get();
  }

  void ListBox::setItemAt(int index, const std::string& value)
  {
    this->items[index].button->setText(value);
  }

  void ListBox::setWrapping(bool wrapping)
  {
    this->wrapping = wrapping;
  }

  bool ListBox::isWrapping() const
  {
    return wrapping;
  }

  void ListBox::clearKeyboardInputBlocker()
  {
    delete this->keyboardInputBlocker;
    this->keyboardInputBlocker = nullptr;
  }

  void ListBox::createKeyboardInputBlocker()
  {
    if (!this->keyboardInputBlocker && this->keyboardBlockType != KeyboardInputBlockType::None)
      this->keyboardInputBlocker = KeyboardInputBlockerFactory::createKeyboardInputBlocker(this->keyboardBlockType);
  }

  int ListBox::getBottomSelectedIndex() const
  {
    int count = getItemCount() - 1;
    // finds the last selected index
    if (!items.empty())
      for (std::vector<ListBoxItem>::const_reverse_iterator it = items.rbegin();
           it != items.rend(); ++it)
      {
        if (it->isSelected())
          return count;
        --count;
      }
    return -1;
  }

  void ListBox::selectRange(int startIndex, int endIndex)
  {
    if (!this->indexExists(startIndex) || !this->indexExists(endIndex) ||
        (startIndex == endIndex && this->getSelectedIndex() == this->getBottomSelectedIndex()))
      return;

    if (startIndex > endIndex)
    {
      int temp = startIndex;
      startIndex = endIndex;
      endIndex = temp;
    }

    this->clearSelectedIndexes(-1);

    for (int i = startIndex; i <= endIndex; ++i)
      items[i].setSelected(true);
  }

  bool ListBox::mouseDrag(const MouseEvent& mouseEvent)
  {
    Widget::mouseDrag(mouseEvent);

    if (mouseEvent.getButton() != MouseButton::LEFT)
      return false;

    int index = this->getIndexAtPoint(mouseEvent.getPosition());

    this->editSelectedIndex(index);

    if (this->getSelectedIndex() != index)
    {
      this->dispatchItemSelect(this->getSelectedIndex());
      this->moveToSelection(index, ScrollMode::InView);
    }
    return true;
  }

  void ListBox::itemMouseDrag(const MouseEvent& mouseEvent)
  {
    if (!this->enableDrag || this->dragIndex == -1)
      return;
    Point absPos = mouseEvent.getSourceWidget()->getAbsolutePosition() + mouseEvent.getPosition();
    Point relPos = absPos - this->getAbsolutePosition();
    int to = this->getIndexAtPoint(relPos);
    if (this->dragIndex != to)
    {
      this->dragItemAt(this->dragIndex, to);
      this->dragIndex = to;
      this->makeSelection(to, this->dragIndex < to ? MoveDirection::Down : MoveDirection::Up);
      this->triggerResize();
    }
  }

  void ListBox::itemMouseDown(const MouseEvent& mouseEvent)
  {
    if (mouseEvent.getButton() != MouseButton::LEFT)
      return;
    if (this->enableDrag)
      this->setDragIndex(this->getIndexOfWidget(mouseEvent.getSourceWidget()));
  }

  void ListBox::itemMouseUp()
  {
    if (!this->enableDrag)
      return;
    this->dragIndex = -1;
  }

  void ListBox::itemMouseDoubleClick(const MouseEvent& mouseEvent)
  {
    // When double-click happens on the listbox line item it's sent here and re-dispatched.
    // However when double-click is dispatched here it also sends an event to itself so
    // to avoid infinite recursion if the source is itself don't re-dispatch
    // the event since it's already in process of dispatching.
    if (mouseEvent.getSourceWidget() == this)
      return;
    if (mouseEvent.getSourceWidget() == &this->itemHolder.horizontalScrollBar ||
        mouseEvent.getSourceWidget() == &this->itemHolder.verticalScrollBar)
      return;
    {
      int itemIndex = this->getIndexOfWidget(mouseEvent.getSourceWidget());
      if (this->indexExists(itemIndex))
        this->dispatchItemDoubleClick(itemIndex);
    }
    this->dispatchDoubleClick(mouseEvent.copyWithNewSource(this));
  }

  void ListBox::itemToggle(const Widget* source, bool leftClick)
  {
    int index = this->getIndexOfWidget(source);
    this->clearSelectedIndexes(index);
    this->moveToSelection(index, ScrollMode::InView);
    this->dispatchItemSelect(this->getSelectedIndex(), leftClick);
  }

  int ListBox::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return false;
    return std::min(this->itemHolder.maximumVerticalSquashSize(), this->getHeight() - this->style.getMinimalHeight());
  }

  int ListBox::maximumHorizontalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return false;
    return std::min(this->itemHolder.maximumHorizontalSquashSize(), this->getWidth() - this->style.getMinimalWidth());
  }

  void ListBox::setKeyboardInputBlockType(KeyboardInputBlockType type)
  {
    if (this->keyboardBlockType == type)
      return;

    this->keyboardBlockType = type;
    if (this->keyboardBlockType == KeyboardInputBlockType::None)
      this->clearKeyboardInputBlocker();
    else if (this->selectByTyping && this->isFocused())
      this->createKeyboardInputBlocker();
  }

  void ListBox::focusGained(TabbedIn tabbedIn)
  {
    Widget::focusGained(tabbedIn);
    if (this->selectByTyping)
      this->createKeyboardInputBlocker();
  }

  void ListBox::focusLost()
  {
    Widget::focusLost();
    this->clearKeyboardInputBlocker();
  }

  void ListBox::reapplySubStyles()
  {
    this->itemHolder.style.setParent(this->style.getScrollPaneStyle());
    for (ListBoxItem& item : this->items)
      item.button->style.setParent(this->style.getItemStyle());
  }

  void ListBox::setSingleSelection()
  {}

  bool ListBox::isSingleSelection() const
  {
    return true;
  }

  void ListBox::setNewItemColor(const agui::Color& color)
  {
    this->newItemColor = color;
  }

  const Color& ListBox::getNewItemColor() const
  {
    return this->newItemColor;
  }

  const ListBoxItem& ListBox::getListItemAt(int index) const
  {
    if (!this->indexExists(index))
      throw agui::Exception("ListItem Not Found");

    return this->items[index];
  }

  void ListBox::setItemTextColor(const agui::Color& color, int index)
  {
    if (!this->indexExists(index))
      throw agui::Exception("ListItem Not Found, Item Color NOT set");

    items[index].button->style.setDefaultFontColor(color);
    items[index].button->style.setHoveredFontColor(color);
    items[index].button->style.setClickedFontColor(color);
    items[index].button->style.setDisabledFontColor(color);
  }

  void ListBox::setItemStrikethrough(bool value, int index)
  {
    if (!this->indexExists(index))
      throw agui::Exception("ListItem Not Found, Item Strikethrough NOT set");

    this->items[index].button->strikethrough = value;
  }

  void ListBox::setItemStyle(int index, const ButtonStyle* itemStyle)
  {
    assert(index < int(this->items.size()));
    this->items[index].button->style.setParent(itemStyle);
  }

  void ListBox::setItemEnabled(int index, bool enabled)
  {
    assert(index < int(this->items.size()));
    this->items[index].button->setEnabled(enabled);
  }

  void ListBox::setItemVisible(int index, bool visible)
  {
    this->items[index].button->setVisible(visible);
  }

  void ListBox::setItemToolTip(int index, const std::string& text)
  {
    assert(index < int(this->items.size()));
    this->items[index].button->setToolTip(text);
  }

  void ListBox::setItemToolTip(int index, std::string&& text)
  {
    assert(index < int(this->items.size()));
    this->items[index].button->setToolTip(std::move(text));
  }

  void ListBox::setItemToolTipCreator(int index, agui::ToolTipCreatorBase* toolTipCreator)
  {
    assert(index < int(this->items.size()));
    this->items[index].button->setToolTipCreator(toolTipCreator);
  }

  void ListBox::setItemRemark(int index, const std::string& text)
  {
    agui::TextButton* button = this->items[index].button.get();
    if (agui::Widget* remark = button->getRemark())
    {
      remark->setText(text);
      return;
    }
    button->addRemark(&agui::label(text));
  }

  bool ListBox::isItemEnabled(int index) const
  {
    assert(index < int(this->items.size()));
    return this->items[index].button->isEnabled();
  }

  bool ListBox::isItemVisible(int index) const
  {
    return this->items[index].button->isVisible();
  }

  bool ListBox::getSelectByTyping() const
  {
    return this->selectByTyping;
  }

  void ListBox::setSelectByTyping(bool value)
  {
    if (this->selectByTyping == value)
      return;

    this->selectByTyping = value;
    if (!this->selectByTyping)
    {
      this->lastKeyTimestamp = 0;
      this->searchText = std::string();
      this->setKeyboardInputBlockType(KeyboardInputBlockType::None);
    }
    else
      this->setKeyboardInputBlockType(KeyboardInputBlockType::AlphaNumerical);
  }

  void ListBox::selectBestSelectedItemByBeginingText(const std::string& text)
  {
    using Match = std::pair<int, uint32_t>;
    if (text.empty() || this->items.empty())
      return;

    Match bestMatch = Match(-1, 0u);
    std::string needle = text;
    std::transform(needle.begin(),
                   needle.end(),
                   needle.begin(),
                   [](char c) { return char(::toupper(c)); });
    for (uint32_t i = 0; i < this->items.size(); ++i)
    {
      std::string haystack = this->items[i].button->getText();
      std::transform(haystack.begin(),
                     haystack.end(),
                     haystack.begin(),
                     [](char c) { return char(::toupper(c)); });
      uint32_t count = 0;
      for (; count < haystack.size() && count < needle.size() && haystack[count] == needle[count]; ++count);
      if (count > bestMatch.second)
        bestMatch = Match(i, count);
    }

    if (bestMatch.first != -1)
    {
      this->editSelectedIndex(bestMatch.first);
      this->moveToSelection(bestMatch.first, ScrollMode::InView);
    }
  }

  void ListBox::setSelectedItem(const std::string& item)
  {
    if (item.empty())
    {
      this->setSelectedIndex(-1);
      return;
    }

    for (size_t i = 0; i < this->items.size(); ++i)
      if (this->items[i].button->getText() == item)
      {
        this->setSelectedIndex(static_cast<int>(i));
        break;
      }
  }

  void ListBox::setSelectNextOnEraseOfSelected(bool value)
  {
    this->selectNextOnEraseOfSelected = value;
  }

  bool ListBox::getSelectNextOnEraseOfSelected() const
  {
    return this->selectNextOnEraseOfSelected;
  }

  void ListBox::selectLastEnabledItem()
  {
    if (!this->items.empty())
      for (int i = int(this->items.size()) - 1; i >= 0; i--)
        if (this->items[i].button->isEnabled())
        {
          this->setSelectedIndex(static_cast<int>(i));
          this->moveToSelection(static_cast<int>(i), ScrollMode::InView);
          break;
        }
  }

  bool ListBox::consumesKeyWhenFocused(KeyEnum key) const
  {
    if (key == KEY_ENTER &&
        Util::anyOf(this->actionListeners, [](const Listener& listener){ return listener.type == Listener::Type::OnConfirm; }))
      return true;

    if (!this->selectByTyping)
      return false;

    if (key == KEY_SPACE)
      return true;

    return key >= KEY_0 &&
           key <= KEY_9 ||
           key >= KEY_A &&
           key <= KEY_Z ||
           key == KEY_UNDERSCORE ||
           key == KEY_SPACE ||
           key == KEY_HYPHEN ||
           key == KEY_PERIOD;
  }

  void ListBox::setEnableDrag(bool value)
  {
    this->enableDrag = value;
    if (!value)
      this->dragIndex = -1;
  }

  bool ListBox::getEnableDrag() const
  {
    return this->enableDrag;
  }

  int ListBox::getVScrollPosition() const
  {
    return this->itemHolder.verticalScrollBar.getValue();
  }

  void ListBox::setVScrollPosition(int position)
  {
    this->itemHolder.verticalScrollBar.setValue(position);
  }

  void ListBox::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    super::setSize(width, height, setSizeInfo);
    setSizeInfo.maximalWidth = this->style.getMaximalWidth();
    this->itemHolder.setSize(this->getWidth(), this->getHeight(), setSizeInfo);
  }

  void ListBox::resizeToContents()
  {
    if (!this->isVerticallyStretchable())
      this->setSize(this->itemHolder.getWidth(), this->itemHolder.getHeight());
    else
      super::resizeToContents();
    }

  int ListBox::requiredWidth() const
  {
    int result = 0;
    for (const ListBoxItem& item : this->items)
      result = std::max(result, item.button->getSizeBeforeStretching().width - item.button->getHorizontalPaddings());
    result += this->itemHolder.verticalScrollBar.getWidth();
    return result;
  }

  bool ListBox::genericSearch(const LowercaseString& filter)
  {
    bool result = super::genericSearch(filter);
    this->reselectAfterSearch(filter);
    return result;
  }

  void ListBox::reselectAfterSearch(const LowercaseString&)
  {
    if (!this->selectAsReactionToSearch)
      return;

    if (this->getSelectedIndex() < 0 ||
        this->getSelectedIndex() >= int(this->items.size()) ||
        this->items[this->getSelectedIndex()].button->isHiddenBySearch())
    {
      for (int i = 0; i < int(this->items.size()); ++i)
        if (this->items[i].button->isVisible())
        {
          this->editSelectedIndex(i);
          return;
        }
      this->editSelectedIndex(-1);
    }
  }

  void ListBox::onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemSelect);
    this->actionListeners.back().onItemSelect = [callback](int index, bool leftClick){ (void)(leftClick); callback(index); };
  }

  void ListBox::onItemSelect(GenericTargetable* owner, std::function<void(int index, bool leftClick)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemSelect);
    this->actionListeners.back().onItemSelect = std::move(callback);
  }

  void ListBox::onItemDoubleClick(GenericTargetable* owner, std::function<void(int index)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemDoubleClick);
    this->actionListeners.back().onItemDoubleClick = std::move(callback);
  }

  void ListBox::editSelectedIndex(int index)
  {
    if (this->getSelectedIndex() == index)
      return;

    this->clearSelectedIndexes(index);
    this->setSelectedIndex(index);
    this->dispatchItemSelect(index);
  }
}

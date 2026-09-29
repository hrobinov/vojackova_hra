#include "Agui/MouseEvent.hpp"
#include "Agui/Widget/TableWithSelection.hpp"
#include "Agui/Widget/HorizontalFlow.hpp"
#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/TextButton.hpp"
#include "Agui/Font.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/EventDispatchHelper.hpp"
#include <cassert>

namespace agui
{
  TableWithSelection::TableWithSelection(const TableStyle* parentStyle)
    : Table(parentStyle)
  {
    this->mouseButtonFilter = agui::MouseButton::LEFT;
  }

  void TableWithSelection::keyAction(ExtendedKeyEnum key, bool shift)
  {
    (void)(shift);
    switch (key)
    {
      case EXT_KEY_UP:
        if (this->hasSelectedIndex() && this->selectedIndex != 0)
          this->editSelectedIndex(this->selectedIndex - 1, -1);
        break;
      case EXT_KEY_DOWN:
        if (this->hasSelectedIndex() && this->selectedIndex != this->getRowCount() - 1)
          this->editSelectedIndex(this->selectedIndex + 1, -1);
        break;
      case EXT_KEY_HOME:
        this->editSelectedIndex(this->hasHeaders ? 1 : 0, -1);
        break;
      case EXT_KEY_END:
        this->editSelectedIndex(this->getRowCount() - 1, -1);
        break;
      default:
        // skip scrolling below
        return;
    }
  }

  void TableWithSelection::updateScrollpaneVisibility()
  {
    int rowHeight = 0;
    const uint32_t selectedIndex = this->getSelectedIndex();
    int rowVerticalPosition = this->getRowVerticalPosition(selectedIndex, rowHeight);
    this->keepVerticallyVisibleInScrollPane(rowVerticalPosition, rowHeight + this->style.getVerticalSpacing().getSpacingAfter(selectedIndex));
  }

  uint32_t TableWithSelection::getHeightSumForIndex(uint32_t index) const
  {
    // no top padding since this is called from paintComponent
    uint32_t heightSum = this->getTopPadding();
    if (const agui::ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      heightSum += columnGraphicalSet->getTopBorder();

    int verticalCellPadding = this->style.getTopCellPadding() + this->style.getBottomCellPadding();
    for (uint32_t i = 0; i < index && i < this->getRowCount(); i++)
    {
      heightSum += this->rowHeights[i];
      heightSum += verticalCellPadding;
      if (i != 0)
        heightSum += this->style.getVerticalSpacing().getSpacingAfter(i - 1);
    }
    return heightSum + this->style.getVerticalSpacing().getSpacingAfter(index) / 2;
  }

  void TableWithSelection::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    // draw selection / hover visualization
    int heightAdjustment = this->style.getVerticalSpacing().getSpacingAfter(this->selectedIndex) + this->style.getTopCellPadding() + this->style.getBottomCellPadding();

    if (!this->style.getSelectedGraphicalSet())
    {
      if (this->hasSelectedIndex() &&
          this->selectedIndex < this->rowHeights.size() &&
          (!this->hasHeaders || this->selectedIndex > 0))
        paintEvent.graphics()->drawFilledRectangle(Rectangle(Point (0, this->getHeightSumForIndex(this->selectedIndex)),
                                                             Dimension(this->getWidth(), heightAdjustment + this->rowHeights[this->selectedIndex])),
                                                   this->style.getSelectedRowColor());
    }

    if (!this->style.getHoveredGraphicalSet())
    {
      if (this->hasHoverIndex() &&
          this->hoverIndex != this->selectedIndex &&
          this->hoverIndex < this->rowHeights.size() &&
          (!this->hasHeaders || this->hoverIndex > 0))
        paintEvent.graphics()->drawFilledRectangle(Rectangle(Point (0, this->getHeightSumForIndex(this->hoverIndex)),
                                                             Dimension(this->getWidth(), heightAdjustment + this->rowHeights[this->hoverIndex])),
                                                   this->style.getHoveredRowColor());
    }

    super::paintBackground(paintEvent, absolutePosition);
  }

  bool TableWithSelection::mouseWheelDown(const MouseEvent& mouseEvent)
  {
    bool res = super::mouseWheelDown(mouseEvent);
    if (!this->enableSelection)
      return res;
    this->editHoverIndex(this->getRowIndexAtPoint(Point(0, this->lastMouseY)));
    return true;
  }

  bool TableWithSelection::mouseWheelUp(const MouseEvent& mouseEvent)
  {
    bool res = super::mouseWheelUp(mouseEvent);
    if (!this->enableSelection)
      return res;
    this->editHoverIndex(this->getRowIndexAtPoint(Point(0, this->lastMouseY)));
    return true;
  }

  bool TableWithSelection::mouseDown(const MouseEvent& mouseEvent)
  {
    if (!this->enableSelection)
      return false;
    uint32_t rowIndex = this->getRowIndexAtPoint(mouseEvent.getPosition());
    this->selectedIsClicked = (rowIndex == this->selectedIndex); // aids in detecting selection double-clicks
    if (rowIndex != 0 || !this->hasHeaders)
      this->editSelectedIndex(this->getRowIndexAtPoint(mouseEvent.getPosition()), this->getColumnIndexAtPoint(mouseEvent.getPosition()));
    this->dispatchItemSelect(this->getSelectedIndex());
    this->hoverIsClicked = true;
    return true;
  }

  bool TableWithSelection::mouseDoubleClick(const MouseEvent&)
  {
    if (!this->enableSelection)
      return false;
    if (this->selectedIsClicked)
      this->dispatchUserDoubleClickedSelection(this->selectedIndex);
    return true;
  }

  bool TableWithSelection::mouseMove(const MouseEvent& mouseEvent)
  {
    bool res = super::mouseMove(mouseEvent);
    if (!this->enableSelection)
      return res;
    if (mouseEvent.getPosition().x >= (int)this->getLeftPadding() + this->getContentWidth() ||
        mouseEvent.getPosition().y >= (int)this->getTopPadding() + this->getContentHeight())
      return false;
    this->lastMouseY = mouseEvent.getPosition().y;
    uint32_t rowIndex = this->getRowIndexAtPoint(mouseEvent.getPosition());
    if (!this->hasHeaders || rowIndex > 0)
      this->editHoverIndex(rowIndex);
    else
      this->editHoverIndex(uint32_t(-1));
    return true;
  }

  bool TableWithSelection::mouseLeave(const MouseEvent& mouseEvent)
  {
    bool res = super::mouseLeave(mouseEvent);
    if (!this->enableSelection)
      return res;
    this->lastMouseY = uint32_t(-1);
    this->editHoverIndex(uint32_t(-1));
    return true;
  }

  agui::Widget* TableWithSelection::getWidgetUnderMouse(Point mousePosition, Point thisAbsolutePosition, Rectangle absolutePositionClip, TransparentValue parentTransparent)
  {
    TransparentValue transparent = transparentOpaqueMask(this->isTransparent(), parentTransparent);
    agui::Widget* result = super::getWidgetUnderMouse(mousePosition, thisAbsolutePosition, absolutePositionClip, transparent);
    if (result)
    {
      if (!this->enableSelection || result->asClickable())
        return result;
      // Make the header widgets clickable otherwise sorting with them is super annoying.
      for (const Header& header : this->headers)
        if (result == header.widget)
          return result;
    }

    return this->getUnderMouseWithTransparency(transparent);
  }

  bool TableWithSelection::mouseUp(const MouseEvent& mouseEvent)
  {
    if (!this->enableSelection)
      return false;
    uint32_t rowIndex = this->getRowIndexAtPoint(mouseEvent.getPosition());
    if (!this->hasHeaders || rowIndex > 0)
      this->editHoverIndex(rowIndex);
    this->hoverIsClicked = false;
    return true;
  }

  bool TableWithSelection::keyDown(const KeyEvent& keyEvent)
  {
    if (!this->enableSelection)
      return false;
    this->keyAction(keyEvent.getExtendedKey(), keyEvent.shift());
    if (keyEvent.getKey() == KEY_SPACE || keyEvent.getKey() == KEY_ENTER)
      this->dispatchItemSelectConfirm(this->getSelectedIndex());
    return true;
  }

  bool TableWithSelection::keyRepeat(const KeyEvent& keyEvent)
  {
    if (!this->enableSelection)
      return false;
    this->keyAction(keyEvent.getExtendedKey(), keyEvent.shift());
    return true;
  }

  void TableWithSelection::add(Widget* widget)
  {
    if (!widget)
      return;
    if (this->getRowIndex(this->getChildCount()) == this->hoverIndex ||
        this->getRowIndex(this->getChildCount()) == this->selectedIndex)
      widget->callRecursively([](agui::Widget* widget){ widget->setParentHovered(true); });
    super::add(widget);
  }

  void TableWithSelection::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    const uint32_t storedHoverIndex = this->hoverIndex;
    const uint32_t storedSelectedIndex = this->selectedIndex;

    this->setSelectedIndex(uint32_t(-1));
    this->setHoverIndex(uint32_t(-1));

    super::remove(widget, keepWidgetAlive);

    if (storedHoverIndex < this->rowHeights.size())
      this->setHoverIndex(storedHoverIndex);
    if (storedSelectedIndex < this->rowHeights.size())
      this->setSelectedIndex(storedSelectedIndex);
  }

  void TableWithSelection::editHoverIndex(uint32_t index)
  {
    // Invalid index
    if (index != uint32_t(-1) && !this->indexExists(index))
      return;

    // Hovering header - don't hover it but make sure hover index clears
    if (this->hasHeaders && index == 0)
      index = uint32_t(-1);

    // Already hovered
    if (this->hoverIndex == index)
      return;

    uint32_t oldIndex = this->hoverIndex;
    this->setHoverIndex(index);
    this->dispatchUserChangedHoverIndex(oldIndex, index);

    if (oldIndex == uint32_t(-1))
    {
      this->getGui()->resetHoverTime();
      this->getGui()->lastHoveredControl.clear();
    }
  }

  void TableWithSelection::editSelectedIndex(uint32_t index, int32_t columnIndex)
  {
    // Invalid index
    if (index != uint32_t(-1) && !this->indexExists(index))
      return;

    // Selecting header
    if (this->hasHeaders && index == 0)
      return;

    // Already selected
    if (this->selectedIndex == index && !this->alwaysFireSelectionChanged)
      return;

    uint32_t oldIndex = this->selectedIndex;
    this->setSelectedIndex(index);
    if (this->getSelectedTableIndex().hasValue)
      this->updateScrollpaneVisibility();
    this->dispatchUserChangedSelection(oldIndex, index, columnIndex);
    this->editHoverIndex(this->getRowIndexAtPoint(Point(0, this->lastMouseY)));
  }

  void TableWithSelection::editSelectedIndexBeforeResize(uint32_t index, int32_t columnIndex)
  {
    this->recalculateRowHeights();
    this->editSelectedIndex(index, columnIndex);
  }

  bool TableWithSelection::getColumnOrdering(uint32_t index) const
  {
    if (index < this->headers.size())
      return this->headers[index].ascending;
    return false;
  }

  void TableWithSelection::setColumnOrdering(uint32_t columnIndex, bool ascending)
  {
    if (this->columnOrderingIndex != uint32_t(-1))
    {
      Header& header = this->headers[this->columnOrderingIndex];
      header.sortingButton->style.setParent(header.ascending ?
                                            this->style.getInactiveColumnOrderingAscendingButtonStyle() :
                                            this->style.getInactiveColumnOrderingDescendingButtonStyle());
    }

    Header& header = this->headers[columnIndex];
    if (header.sortingButton == nullptr)
    {
      header.sortingButton = new agui::TextButton();
      header.sortingButton->onClick(this, [this, columnIndex]() { this->editColumnOrderingIndex(columnIndex); });
      *header.flow << agui::hold(header.sortingButton);
      header.widget->setFor(header.sortingButton);
    }

    this->columnOrderingIndex = columnIndex;
    header.ascending = ascending;
    header.sortingButton->style.setParent(header.ascending ?
                                          this->style.getColumnOrderingAscendingButtonStyle() :
                                          this->style.getColumnOrderingDescendingButtonStyle());
  }

  void TableWithSelection::onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemSelect);
    this->actionListeners.back().onItemSelect = [callback](int index, bool leftClick){ (void)(leftClick); callback(index); };
  }

  void TableWithSelection::onTableOrderingColumnChange(GenericTargetable* owner, std::function<void(TableIndex columnIndex, bool ascending)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnTableOrderingColumnChange);
    this->actionListeners.back().onTableOrderingColumnChange = std::move(callback);
  }

  void TableWithSelection::onTableSelectionChange(GenericTargetable* owner, std::function<void(TableIndex newIndex, TableIndex oldIndex, int32_t columnIndex)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnTableSelectionChange);
    this->actionListeners.back().onTableSelectionChange = std::move(callback);
  }

  void TableWithSelection::onTableHoverChange(GenericTargetable* owner, std::function<void(TableIndex newIndex, TableIndex oldIndex)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnTableHoverChange);
    this->actionListeners.back().onTableHoverChange = std::move(callback);
  }

  void TableWithSelection::onTableSelectionDoubleClick(GenericTargetable* owner, std::function<void(TableIndex index)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnTableSelectionDoubleClick);
    this->actionListeners.back().onTableSelectionDoubleClick = std::move(callback);
  }

  uint32_t TableWithSelection::getRowIndex(uint32_t childIndex) const
  {
    return childIndex / this->getColumnCount();
  }

  void TableWithSelection::editColumnOrderingIndex(uint32_t columnIndex)
  {
    this->setColumnOrdering(columnIndex,
                            this->columnOrderingIndex == columnIndex
                            ? !this->headers[columnIndex].ascending
                            : this->headers[columnIndex].ascending);
    this->dispatchUserChangedOrderingColumn(this->columnOrderingIndex, this->headers[columnIndex].ascending);
  }

  void TableWithSelection::callOnRowRecursive(uint32_t rowIndex, const std::function<void(Widget*)>& callback)
  {
    if (rowIndex == uint32_t(-1))
      return;
    for (uint32_t i = rowIndex * this->getColumnCount(); i < (rowIndex + 1) * this->getColumnCount() && i < this->getChildCount(); ++i)
      this->getChildAt(i)->callRecursively(callback);
  }

  void TableWithSelection::dispatchUserChangedSelection(uint32_t oldIndex, uint32_t newIndex, int32_t columnIndex)
  {
    TableIndex oldEventIndex(oldIndex);
    TableIndex newEventIndex(newIndex);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnTableSelectionChange))
      listener.onTableSelectionChange(newEventIndex, oldEventIndex, columnIndex);
  }

  void TableWithSelection::dispatchUserChangedHoverIndex(uint32_t oldIndex, uint32_t newIndex)
  {
    TableIndex oldEventIndex(oldIndex);
    TableIndex newEventIndex(newIndex);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnTableHoverChange))
      listener.onTableHoverChange(newEventIndex, oldEventIndex);
  }

  void TableWithSelection::dispatchUserChangedOrderingColumn(uint32_t columnIndex, bool ascending)
  {
    TableIndex eventColumnIndex(columnIndex);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnTableOrderingColumnChange))
      listener.onTableOrderingColumnChange(eventColumnIndex, ascending);
  }

  void TableWithSelection::dispatchUserDoubleClickedSelection(uint32_t index)
  {
    TableIndex eventIndex(index);
    for (Listener& listener : EventDispatchHelper(this, this->actionListeners, Listener::Type::OnTableSelectionDoubleClick))
      listener.onTableSelectionDoubleClick(eventIndex);
  }

  uint32_t TableWithSelection::getRowIndexAtPoint(const Point& p) const
  {
    int y = p.y;

    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      y -= columnGraphicalSet->getTopBorder();

    if (y < 0)
      return uint32_t(-1);

    int verticalCellPadding = this->style.getTopCellPadding() + this->style.getBottomCellPadding();

    uint32_t rowIndex;
    for (rowIndex = 0; rowIndex < this->getRowCount(); ++rowIndex)
    {
      y -= this->rowHeights[rowIndex];
      y -= verticalCellPadding;
      y -= this->style.getVerticalSpacing().getSpacingAfter(rowIndex);
      if (y <= 0)
        break;
    }

    if (rowIndex >= this->getRowCount())
      return uint32_t(-1);

    if (this->indexExists(rowIndex))
      return rowIndex;
    return uint32_t(-1);
  }

  int TableWithSelection::getRowVerticalPosition(uint32_t rowIndex, int& rowHeight) const
  {
    uint32_t result = this->getTopPadding();
    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      result += columnGraphicalSet->getTopBorder();

    if (rowIndex == uint32_t(-1))
      return result;

    if (this->hasHeaders)
      rowIndex++;
    if (rowIndex == 0 || this->rowHeights.empty())
      return result;

    int verticalCellPadding = this->style.getTopCellPadding() + this->style.getBottomCellPadding();

    for (uint32_t i = 0; i < std::min(size_t(rowIndex), this->rowHeights.size()); i++)
      result += this->rowHeights[i] + verticalCellPadding + this->style.getVerticalSpacing().getSpacingAfter(i);

    if (rowIndex >= this->rowHeights.size())
      rowIndex = uint32_t(this->rowHeights.size()) - 1u;

    rowHeight = this->rowHeights[rowIndex] + verticalCellPadding;

    // we want position in the middle of the row
    return result + rowHeight / 2;
  }

  uint32_t TableWithSelection::getColumnIndexAtPoint(const Point& p) const
  {
    int x = p.x;

    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      x -= columnGraphicalSet->getLeftBorder();

    if (x < 0)
      return uint32_t(-1);

    std::vector<int> columnWidths = getColumnWidths(this->columnCount);
    uint32_t columnIndex = 0;
    int horizontalCellPadding = this->style.getLeftCellPadding() + this->style.getRightCellPadding();

    for (uint32_t i = 0; i < columnWidths.size(); i++)
    {
      x -= columnWidths[i];
      x -= horizontalCellPadding;
      x -= this->style.getHorizontalSpacing().getSpacingAfter(i);
      if (x <= 0)
        break;
      columnIndex++;
    }

    if (columnIndex >= columnWidths.size())
      return uint32_t(-1);
    return columnIndex;
  }

  void TableWithSelection::clear()
  {
    super::clear();
    this->headers.clear();
    this->setSelectedIndex(uint32_t(-1));
    this->lastMouseY = uint32_t(-1);
    this->setHoverIndex(uint32_t(-1));
  }

  void TableWithSelection::clearApartHeader()
  {
    this->setSelectedIndex(uint32_t(-1));
    this->setHoverIndex(uint32_t(-1));


    uint32_t visibleChildrenCount = 0;
    for (Widget* widget: VisibleChildren(this))
    {
      (void)(widget);
      ++visibleChildrenCount;
    }
    while (visibleChildrenCount > this->getColumnCount())
    {
      Widget* lastChild = this->getChildAt(this->getChildCount() - 1u);
      this->getChildren().pop_back();
      this->handleChildRemoved(lastChild);
      --visibleChildrenCount;
    }
    this->visibleChildren.clear();
    this->triggerResize();
  }

  void TableWithSelection::addHeader(Widget& widget, bool sortable, bool centered, bool clickOnWidget)
  {
    this->setHasHeaders(true);

    Header header;
    header.widget = &widget;
    header.flow = new agui::HorizontalFlow();
    header.flow->style.setVerticalAlign(agui::VerticalAlign::Center);

    if (centered)
      header.flow->style.setHorizontalAlign(agui::HorizontalAlign::Center);

    header.flow->autoDestructWhenRemovedFromParent();
    if (sortable)
    {
      header.sortingButton = new agui::TextButton(this->style.getInactiveColumnOrderingDescendingButtonStyle());
      int size = int(this->headers.size());
      header.sortingButton->onClick(this, [this, size]() { this->editColumnOrderingIndex(size); });
      if (clickOnWidget)
      {
        header.widget->onClick(this, [this, size]() { this->editColumnOrderingIndex(size); });
        header.flow->onClick(this, [this, size]() { this->editColumnOrderingIndex(size); });
        header.widget->setFor(header.sortingButton);
      }
      *header.flow << agui::hold(header.sortingButton);
    }

    *this << (*header.flow << header.widget);
    this->headers.push_back(header);
  }

  void TableWithSelection::adjustHeaderMargins(int16_t top, int16_t right, int16_t bottom, int16_t left)
  {
    for (Header& header : this->headers)
    {
      header.flow->setTopMargin(header.flow->getTopMargin() + top);
      header.flow->setRightMargin(header.flow->getRightMargin() + right);
      header.flow->setBottomMargin(header.flow->getBottomMargin() + bottom);
      header.flow->setLeftMargin(header.flow->getLeftMargin() + left);
    }
  }

  void TableWithSelection::reapplySubStyles()
  {
    for (Header& header : this->headers)
      header.sortingButton->style.setParent(header.ascending ?
                                            this->style.getInactiveColumnOrderingAscendingButtonStyle() :
                                            this->style.getInactiveColumnOrderingDescendingButtonStyle());

    if (this->columnOrderingIndex != uint32_t(-1))
    {
      Header& header = this->headers[this->columnOrderingIndex];
      header.sortingButton->style.setParent(header.ascending ?
                                            this->style.getColumnOrderingAscendingButtonStyle() :
                                            this->style.getColumnOrderingDescendingButtonStyle());
    }

    super::reapplySubStyles();
  }

  void TableWithSelection::setHoverIndex(uint32_t index)
  {
    if (this->hoverIndex == index)
      return;

    assert(index == uint32_t(-1) || index < this->rowHeights.size());
    if (this->hoverIndex != this->selectedIndex)
      this->callOnRowRecursive(this->hoverIndex, [](agui::Widget* widget){ widget->setParentHovered(false); });
    this->hoverIndex = index;
    if (this->hoverIndex != this->selectedIndex)
      this->callOnRowRecursive(this->hoverIndex, [](agui::Widget* widget){ widget->setParentHovered(true); });
  }

  uint32_t TableWithSelection::getSelectedIndex() const
  {
    if (!this->hasSelectedIndex())
      return this->selectedIndex;
    if (this->hasHeaders)
      return this->selectedIndex - 1;
    return this->selectedIndex;
  }

  void TableWithSelection::setSelectedIndex(uint32_t index)
  {
    if (this->selectedIndex == index)
      return;

    // this is necessary if there has been widget additions since last layoutChildren
    // the cleanest solution would be to track whether the recalculation is necessary
    this->recalculateRowHeights();

    if (index != uint32_t(-1) && !this->indexExists(index))
      return; // It may happen that the index was "valid" before the call to ::recalculateRowHeights()
              //  in which case, making external code call ::recalculateRowHeights() would do redundant work.

    if (this->hoverIndex != this->selectedIndex)
      this->callOnRowRecursive(this->selectedIndex, [](agui::Widget* widget){ widget->setParentHovered(false); });
    this->selectedIndex = index;
    if (this->hoverIndex != this->selectedIndex &&
        (this->selectedIndex > 0 || this->headers.empty()))
      this->callOnRowRecursive(this->selectedIndex, [](agui::Widget* widget){ widget->setParentHovered(true); });
  }
}

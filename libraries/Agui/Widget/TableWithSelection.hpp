#pragma once
#include "Agui/Widget/Table.hpp"
#include <cassert>
#include <stdint.h>

namespace agui
{
  class HorizontalFlow;
  class TextButton;

  class TableWithSelection : public Table
  {
    using super = Table;
  public:
    TableWithSelection(const TableStyle* parentStyle);

    virtual TableWithSelection* asTableWithSelection() override { return this; }
    virtual const TableWithSelection* asTableWithSelection() const override { return this; }

    virtual void keyAction(ExtendedKeyEnum key, bool shift);
    virtual void updateScrollpaneVisibility();
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;

    virtual bool mouseWheelDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseDoubleClick(const MouseEvent& mouseEvent) override;
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool mouseMove(const MouseEvent& mouseEvent) override;
    virtual bool mouseLeave(const MouseEvent& mouseEvent) override;
    virtual Widget* getWidgetUnderMouse(Point mousePosition, Point relativePosition, Rectangle absolutePositionClip, TransparentValue) override;

    virtual bool keyDown(const KeyEvent& keyEvent) override;
    virtual bool keyRepeat(const KeyEvent& keyEvent) override;

    // when we have tables with headers it is required to use this method to add the actual headers
    // the point is that we add additional space for the potential indicator image to be drawn
    virtual void add(Widget* widget) override;
    virtual void addHeader(Widget& widget, bool sortable = true, bool centered = false, bool clickOnWidget = true);
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive) override;
    virtual void adjustHeaderMargins(int16_t top, int16_t right, int16_t bottom, int16_t left);
    virtual bool isFocusable() const override { return true; }
    virtual void reapplySubStyles() override;

    bool hasHoverIndex() const { return this->hoverIndex != uint32_t(-1); }
    bool hasSelectedIndex() const { return this->selectedIndex != uint32_t(-1); }
    // takes headers into account (so rawSelectedIndex 1 with headers returns as 0)
    uint32_t getSelectedIndex() const;
    TableIndex getSelectedTableIndex() const { return TableIndex(this->selectedIndex); }
    TableIndex getHoveredTableIndex() const { return TableIndex(this->hoverIndex); }
    virtual uint32_t getRawSelectedIndex() const override { return this->selectedIndex; }
    virtual uint32_t getRawHoveredIndex() const override { return this->hoverIndex; }
    virtual bool getHoverIsClicked() const override { return this->hoverIsClicked; }
    void setSelectedIndex(uint32_t index);
    void editHoverIndex(uint32_t index);
    // Works as if the user made the selection - triggers the ActionListener events.
    void editSelectedIndex(uint32_t index, int32_t columnIndex);
    // Same as ::editSelectedIndex but handles if you change the table contents and call this before waiting for resize
    void editSelectedIndexBeforeResize(uint32_t index, int32_t columnIndex);
    void editColumnOrderingIndex(uint32_t columnIndex);
    uint32_t getColumnOrderingIndex() const { return this->columnOrderingIndex; }
    bool getColumnOrdering(uint32_t index) const;
    void setColumnOrdering(uint32_t columnIndex, bool ascending);
    void onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback);
    void onTableOrderingColumnChange(GenericTargetable* owner, std::function<void(TableIndex columnIndex, bool ascending)> callback);
    void onTableSelectionChange(GenericTargetable* owner, std::function<void(TableIndex newIndex, TableIndex oldIndex, int32_t columnIndex)> callback);
    void onTableHoverChange(GenericTargetable* owner, std::function<void(TableIndex newIndex, TableIndex oldIndex)> callback);
    void onTableSelectionDoubleClick(GenericTargetable* owner, std::function<void(TableIndex index)> callback);
    uint32_t getRowIndex(uint32_t childIndex) const;

    virtual void clear() override;
    virtual void clearApartHeader();
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override
    {
      super::layoutChildren(setSizeInfo);
      this->recalculateRowHeights();
    }
  private:
    void callOnRowRecursive(uint32_t rowIndex, const std::function<void(Widget*)>& callback);
    void dispatchUserChangedSelection(uint32_t oldIndex, uint32_t newIndex, int32_t columnIndex);
    void dispatchUserChangedHoverIndex(uint32_t oldIndex, uint32_t newIndex);
    void dispatchUserChangedOrderingColumn(uint32_t columnIndex, bool ascending);
    void dispatchUserDoubleClickedSelection(uint32_t index);
    // think about scrolling
    uint32_t getRowCount() const { return uint32_t(this->rowHeights.size()); }
    bool indexExists(uint32_t index) const { return index != uint32_t(-1) && index < this->getRowCount(); }
    void setHoverIndex(uint32_t index);
    uint32_t getRowIndexAtPoint(const Point& p) const;
    uint32_t getColumnIndexAtPoint(const Point& p) const;
    void recalculateRowHeights()
    {
      LayoutingContext context(ReactionToSetSize::False, *this);
      this->calculateRowHeights(context);
    }
    uint32_t getHeightSumForIndex(uint32_t index) const;
    int getRowVerticalPosition(uint32_t rowIndex, int& rowHeight) const;

    uint32_t selectedIndex = uint32_t(-1);
    uint32_t hoverIndex = uint32_t(-1);
    bool hoverIsClicked = false;
    uint32_t lastMouseY = uint32_t(-1);
    bool selectedIsClicked = false; // indicates that the selected row was clicked twice in a row

    uint32_t columnOrderingIndex = uint32_t(-1);
    struct Header
    {
      HorizontalFlow* flow = nullptr;
      Widget* widget = nullptr;
      TextButton* sortingButton = nullptr;
      bool ascending = false;
    };
    std::vector<Header> headers;

    bool enableSelection = true;
  public:
    TableWithSelection& setEnableSelection(bool value) { this->enableSelection = value; return *this; }
    bool getEnableSelection() { return this->enableSelection; }

    bool alwaysFireSelectionChanged = false;
  };
}

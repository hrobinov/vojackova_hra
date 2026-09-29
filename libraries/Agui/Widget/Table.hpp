#pragma once
#include "Agui/Layout.hpp"
#include "Agui/Pusher.hpp"
#include "Agui/Widget/TableStyle.hpp"
namespace agui { class TableConnector; }

namespace agui
{
  /** Allows to layout widgets in table, row/column dimensions are dependent on the
   * size of the biggest widget in the corresponding row/column, as in HTML table.
   * Items in the cell have vertical centering.
   * @TODO rowspan/colspan */
  class Table : public Layout
  {
    using super = Layout;
    using BorderFlagType = uint16_t;
  public:
    struct LayoutingContext
    {
      LayoutingContext(ReactionToSetSize reactionToSetSize, const Table& table);

      ReactionToSetSize reactionToSetSize = ReactionToSetSize::False;
      uint32_t childrenCount = 0;
      int cellRightBorder = 0;
      int cellLeftBorder = 0;
      int cellTopBorder = 0;
      int cellBottomBorder = 0;
      int startingLocationX = 0;
      int startingLocationY = 0;

      int topCellPadding = 0;
      int rightCellPadding = 0;
      int bottomCellPadding = 0;
      int leftCellPadding = 0;

      int horizontalCellPadding = 0;
      int verticalCellPadding = 0;
      int borderWidth = 0;
      TableStyle::Spacings horizontalSpacing;
      TableStyle::Spacings verticalSpacing;
      int horizontalBorderInnerWidth = 0;
      int verticalBorderInnerWidth = 0;

      int horizontalCellExtraSpace = 0; // context.cellLeftBorder + context.cellRightBorder + context.horizontalCellPadding;
      int verticalCellExtraSpace = 0; // context.cellTopBorder + context.cellBottomBorder + context.verticalCellPadding;
      SetSizeInfo setSizeInfo;
    };

    class iterator
    {
    public:
      iterator(Table& table) : table(&table), position(table.begin()) { this->nextValid(); }
      iterator& operator*() { return *this; }
      void operator++() { ++this->position; ++this->x; if (this->x == int(this->table->getColumnCount())) { ++this->y; this->x = 0; } this->nextValid(); }
      void operator=(const iterator& other) { this->table = other.table; this->x = other.x; this->y = other.y; this->position = other.position; }
      bool operator!=(const iterator& other) const { return this->position != other.position; }
      bool operator!=(const WidgetArray::iterator& other) const { return this->position != other; }
      Widget& widget() { return **position; }
    private:

      void nextValid() { while (this->position != this->table->end() && !this->widget().isVisible()) ++this->position; }
    public:
      int x = 0, y = 0;
    private:
      Table* table;
      WidgetArray::iterator position;
    };

    class const_iterator
    {
    public:
      const_iterator(const Table& table) : table(&table), position(table.begin()) { this->nextValid(); }
      const_iterator& operator*() { return *this; }
      void operator++() { ++this->position; ++this->x; if (this->x == int(this->table->getColumnCount())) { ++this->y; this->x = 0; } this->nextValid(); }
      void operator=(const const_iterator& other) { this->table = other.table; this->x = other.x; this->y = other.y; this->position = other.position; }
      bool operator!=(const const_iterator& other) const { return this->position != other.position; }
      bool operator!=(const WidgetArray::const_iterator& other) const { return this->position != other; }
      Widget& widget() { return **position; }
    private:

      void nextValid() { while (this->position != this->table->end() && !this->widget().isVisible()) ++this->position; }
    public:
      int x = 0, y = 0;
    private:
      const Table* table;
      WidgetArray::const_iterator position;
    };

    class TableIterator
    {
    public:
      TableIterator(Table& table) : table(table) {}
      iterator begin() { return iterator(this->table); }
      WidgetArray::iterator end() { return this->table.end(); }

      Table& table;
    };

    class ConstTableIterator
    {
    public:
      ConstTableIterator(const Table& table) : table(table) {}
      const_iterator begin() { return const_iterator(this->table); }
      WidgetArray::const_iterator end() { return this->table.end(); }

      const Table& table;
    };

    Table(const TableStyle* parentStyle = &Table::defaultStyle);
    Table(uint32_t columnCount, const TableStyle* parentStyle = &Table::defaultStyle);
    virtual ~Table();
    std::vector<int> getColumnWidths(int columnCount) const;
    virtual void setColumnCount(int columnCount); // Sets the number of columns expected to have. This can never be zero
    Table& operator()(int columnCount) { this->setColumnCount(columnCount); return *this; } // fast way to specify columns in the structure
    virtual void setVerticalCentering(bool value);
    virtual uint32_t getColumnCount() const; // @return The number of columns in the grid or zero if unknown.
    virtual bool getVerticalCentering() const;
    virtual uint32_t getRawSelectedIndex() const { return uint32_t(-1); }
    virtual uint32_t getRawHoveredIndex() const { return uint32_t(-1); }
    virtual bool getHoverIsClicked() const { return false; }
    void setHasHeaders(bool value) { this->hasHeaders = value; }
    void setDrawVerticalLines(bool drawVerticalLines) { this->drawVerticalLines = drawVerticalLines; }
    void setDrawHorizontalLineAfterHeaders(bool value) { this->drawHorizontalLineAfterHeaders = value; }
    void setDrawHorizontalLines(bool drawHorizontalLines) { this->drawHorizontalLines = drawHorizontalLines; }
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    int calculateExpectedHeight(int numSlots, int slotHeight) const;
    agui::Table& setUnifyWidth(bool unifyWidth);
    int getRowYPosition(uint32_t row) const;
    void unifyColumnWidthsWith(Table* table);
    virtual void logic(double) override;
    virtual Widget* getWidgetUnderMouse(Point mousePosition, Point relativePosition, Rectangle absolutePositionClip, TransparentValue) override;
  private:
    Widget* getChildUnderMouseWithBinaryRowSearch(Point mousePosition, Point thisAbsolutePosition, Rectangle absolutePositionClip);
  public:
    virtual void clear() override;
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False) override;
    Table& alignColumn(int columnIndex, agui::AreaAlign align);
    void setBordersToDraw(BorderFlagType borderFlags) { this->bordersToDraw = borderFlags; }
    Table& operator<<(const HorizontalPusher& hPusher);
    Table& operator<<(Widget& other) { this->add(&other); return *this; }
    Table& operator<<(Widget* other) { this->add(other); return *this; }
    template<class T> requires std::is_base_of_v<Widget, T>
    Table& operator<<(std::unique_ptr<T>& other) { this->add(other.get()); return *this; }
    Table& operator<<(const Empty& empty) { Widget::operator<<(empty); return *this; }
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;
    void updateUsesBorder();

    uint32_t getRowCount() const;
    void setAllowDecreaseColumnWidths(bool allow);
  protected:
    void calculateRowHeights(const LayoutingContext& context);
    virtual void layoutChildren(SetSizeInfo setSizeInfo) override;
    int calculateContentHeight(const LayoutingContext& context) const;
  public:
    static TableStyle defaultStyle;
    virtual Style* getStyle() override { return &this->style; }
    virtual TransparentValue isTransparent() const override;
  private:
    // positions of points where vertical borders (between columns) should be drawn
    std::vector<int> getVerticalBorderPositions(int borderWidth) const;
    // positions of points where horizontal borders (between rows) should be drawn
    std::vector<int> getHorizontalBorderPositions(int borderWidth, bool borderOnTop) const;

    void tryToDrawVerticalLines(const PaintEvent& paintEvent);
    void tryToDrawHorizontalLines(const PaintEvent& paintEvent);

    // All these functions work with BorderImageSet, i.e. different sprites to what tryToDrawVerticalLines etc. use.
    // NOTE: used borders override table outer padding or row/column spacing for their use. Cell paddings remain available.
    void drawBorder(const PaintEvent& paintEvent); // Draw borders according to flags, using functions below.
    void drawBorderGrid(const PaintEvent& paintEvent); // full grid
    void drawOuterBorder(const PaintEvent& paintEvent); // complete outer border
    void drawOuterBorderTs(const PaintEvent& paintEvent); // connectors to inner borders
    void drawOuterBorderTopTs(const PaintEvent& paintEvent); // connectors to inner borders
    void drawOuterBorderTop(const PaintEvent& paintEvent); // one border line, with ends
    void drawOuterBorderRightTs(const PaintEvent& paintEvent); // connectors to inner borders
    void drawOuterBorderRight(const PaintEvent& paintEvent); // one border line, with ends
    void drawOuterBorderBottomTs(const PaintEvent& paintEvent); // connectors to inner borders
    void drawOuterBorderBottom(const PaintEvent& paintEvent); // one border line, with ends
    void drawOuterBorderLeftTs(const PaintEvent& paintEvent); // connectors to inner borders
    void drawOuterBorderLeft(const PaintEvent& paintEvent); // one border line, with ends
    void drawOuterBorderTopLeftCorner(const PaintEvent& paintEvent);
    void drawOuterBorderTopRightCorner(const PaintEvent& paintEvent);
    void drawOuterBorderBottomRightCorner(const PaintEvent& paintEvent);
    void drawOuterBorderBottomLeftCorner(const PaintEvent& paintEvent);
    void drawInnerGrid(const PaintEvent& paintEvent, bool withEnds = true); // row and column separators
    void drawInnerHorizontalBorders(const PaintEvent& paintEvent, bool withEnds = true); // row separators
    void drawInnerVerticalBorders(const PaintEvent& paintEvent, bool withEnds = true); // column separators
    void drawInnerCrosses(const PaintEvent& paintEvent); // crosses to correctly connect row and column separators
    void drawInnerHorizontalFirstBorder(const PaintEvent& paintEvent); // one line between first and second row

    void stretchColumnWidthsToSize(const LayoutingContext& context);
    int getBorderWidth() const;
    void resolveColumnWidths(const LayoutingContext& context);
    void resolveRowHeights(const LayoutingContext& context);
    static int sumSpacingsWithOptionalInnerBorder(const TableStyle::Spacings& spacings, int maxIndex, int optionalInnerBorder);

  public:
    TableStyle style;

    static const BorderFlagType NO_BORDER = 0;
    static const BorderFlagType INNER_VERTICAL_BORDERS = 1 << 0;
    static const BorderFlagType INNER_HORIZONTAL_BORDERS = 1 << 1;
    static const BorderFlagType TOP_BORDER = 1 << 2;
    static const BorderFlagType RIGHT_BORDER = 1 << 3;
    static const BorderFlagType BOTTOM_BORDER = 1 << 4;
    static const BorderFlagType LEFT_BORDER = 1 << 5;
    static const BorderFlagType UNDER_FIRST_ROW_BORDER = 1 << 6; // underline under title row
    static const BorderFlagType INNER_GRID_BORDER = INNER_VERTICAL_BORDERS | INNER_HORIZONTAL_BORDERS;
    static const BorderFlagType OUTER_BORDER = TOP_BORDER | RIGHT_BORDER | BOTTOM_BORDER | LEFT_BORDER;
    static const BorderFlagType FULL_GRID_BORDER = INNER_GRID_BORDER | OUTER_BORDER;

    BorderFlagType bordersToDraw = NO_BORDER;
  private:
    bool verticalCentering = true;
    bool drawVerticalLines = false; // If vertical lines dividing individual columns should be drawn
    bool drawHorizontalLines = false;
    bool drawHorizontalLineAfterHeaders = false; // only draw line under headers row
    bool unifyWidth = false;
    TableConnector* connector = nullptr;
  protected:
    std::vector<Widget*> visibleChildren;
  public:
    std::vector<int> columnWidths;
    std::vector<int> rowHeights;
  protected:
    bool dontDecreaseColumnWidths = false;
    bool hasHeaders = false;
    uint32_t columnCount = 1;
  };
}

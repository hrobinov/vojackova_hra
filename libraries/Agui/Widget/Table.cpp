#include "Agui/Widget/EmptyWidget.hpp"
#include "Agui/Widget/Table.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Pusher.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/TableConnector.hpp"
#include "Agui/SquashCalculator.hpp"
#include <algorithm>
#include <cassert>
#include <math.h>

namespace agui
{
  TableStyle Table::defaultStyle;

  Table::Table(const TableStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &Table::defaultStyle)
  {
    this->updateUsesBorder();
  }

  Table::Table(uint32_t columnCount, const TableStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &Table::defaultStyle)
    , columnCount(columnCount)
  {
    this->updateUsesBorder();
  }

  Table::~Table()
  {
    if (this->connector)
      this->connector->remove(this);
  }

  void Table::resolveColumnWidths(const LayoutingContext& context)
  {
    if (this->connector)
    {
      if (!this->connector->isResizeResolved())
        this->connector->resolveResize();

      if (this->dontDecreaseColumnWidths && this->columnWidths.size() == this->getColumnCount())
      {
        std::vector<int> tempWidths = this->connector->getColumnWidths();
        for (uint32_t i = 0; i < tempWidths.size(); ++i)
          this->columnWidths[i] = std::max(this->columnWidths[i], tempWidths[i]);
      }
      else
        this->columnWidths = this->connector->getColumnWidths();
    }
    else
    {
      if (this->dontDecreaseColumnWidths && this->columnWidths.size() == this->getColumnCount())
      {
        std::vector<int> tempWidths = this->getColumnWidths(this->columnCount);
        for (uint32_t i = 0; i < tempWidths.size(); ++i)
          this->columnWidths[i] = std::max(this->columnWidths[i], tempWidths[i]);
      }
      else
        this->columnWidths = this->getColumnWidths(this->columnCount);
    }

    this->stretchColumnWidthsToSize(context);
  }

  void Table::resolveRowHeights(const LayoutingContext& context)
  {
     this->calculateRowHeights(context);

    // squash vertically
    {
      int contentHeight = this->calculateContentHeight(context);
      int targetHeight = contentHeight;
      if (context.reactionToSetSize)
        targetHeight = this->getContentHeight();
      else if (int maximalHeight = this->style.getMaximalHeight())
        targetHeight = maximalHeight;
      if (contentHeight > targetHeight)
      {
        SquashCalculator rowSquashingCalculator(GuiDirection::Vertical);

        int neededHeight = targetHeight;

        //collect the sizes
        for (Table::iterator it : TableIterator(*this))
        {
          if (it.x == 0)
          {
            rowSquashingCalculator.add(it.widget());
            if (it.y != 0)
              neededHeight -= std::max(context.verticalSpacing.getSpacingAfter(uint32_t(it.y) - 1), context.verticalBorderInnerWidth);
            neededHeight -= context.verticalCellExtraSpace;
          }
          else
            rowSquashingCalculator.merge(it.widget());
        }

        // calculate the squashing
        rowSquashingCalculator.calculate(neededHeight);

        // apply the squashing
        for (Table::iterator it : TableIterator(*this))
        {
          int originalWidgetWidth = it.widget().getWidth();
          if (it.widget().getHeight() > rowSquashingCalculator.items[it.y].resultSize)
            it.widget().setSize(it.widget().getWidth(), rowSquashingCalculator.items[it.y].resultSize, SetSizeInfo(context.setSizeInfo.horizontal, Change::Squashing));
          if (originalWidgetWidth < it.widget().getWidth() && it.x < int(this->columnWidths.size()) && it.widget().getWidth() > this->columnWidths[it.x])
          {
            this->columnWidths[it.x] = it.widget().getWidth();
            if (int maximalColumnWidth = this->style.getColumnWidths()->getItem(it.x).maximalWidth)
              this->columnWidths[it.x] = std::min(this->columnWidths[it.x], maximalColumnWidth);
          }
        }
        this->calculateRowHeights(context);
      }
    }
  }

  int Table::sumSpacingsWithOptionalInnerBorder(const TableStyle::Spacings& spacings, const int maxIndex, const int optionalInnerBorder)
  {
    if (maxIndex <= 0)
      return 0;
    if (optionalInnerBorder == 0)
      return spacings.sumSpacingsBetween(0, int32_t(maxIndex));
    if (spacings.uniformSpacing)
      return maxIndex * std::max(*spacings.uniformSpacing, optionalInnerBorder);

    int sum  = 0;
    const std::vector<int>& spacingVector = spacings.getSpacings();
    for (uint32_t i = 0; i < uint32_t(maxIndex) && i < spacingVector.size(); ++i)
      sum += std::max(spacingVector[i], optionalInnerBorder);
    if (uint32_t(spacingVector.size()) < uint32_t(maxIndex))
      sum += (uint32_t(maxIndex) - uint32_t(spacingVector.size())) * optionalInnerBorder;
    return sum;
  }

  void Table::setAllowDecreaseColumnWidths(bool allow)
  {
    this->dontDecreaseColumnWidths = !allow;
  }

  void Table::layoutChildren(SetSizeInfo setSizeInfo)
  {
    this->visibleChildren.clear();

    LayoutingContext context(setSizeInfo.reactionToSetSize, *this);
    context.setSizeInfo = setSizeInfo;

    if (context.childrenCount == 0 && !this->style.getWideAsColumnCount())
    {
      Widget::setSize(0, 0);
      // Resolve heights/widths (which will resolve to zero) otherwise rendering goes wrong using the old values.
      this->resolveRowHeights(context);
      this->resolveColumnWidths(context);
      return;
    }

    // I first resolve heights, as it is quite possible, that changing (squashing) height changes width, which invalidates the
    // resolve column result values.

    // The problem can obviously appear in the opposite dimension as well (squashing horizontally increases/changes vertical size),
    // in that case (if it happens to be an issue ever), we need to detect it, and re-resolve heights again after these 2.
    this->resolveRowHeights(context);
    this->resolveColumnWidths(context);

    bool resolveHeights = false;
    int maxXCount = 0;

    int locationY = context.startingLocationY;
    int locationX = context.startingLocationX;

    // this positions all the children widgets
    for (Table::iterator it : TableIterator(*this))
    {
      maxXCount = std::max(maxXCount, it.x + 1);
      this->visibleChildren.push_back(&it.widget());

      // next row
      if (it.x == 0 && it.y != 0)
      {
        locationX = context.startingLocationX;
        locationY += this->rowHeights[it.y - 1] +
                     std::max(context.verticalSpacing.getSpacingAfter(uint32_t(it.y - 1)),
                              context.verticalBorderInnerWidth) +
                     context.verticalCellPadding;
      }

      if (it.x != 0)
        locationX += this->columnWidths[it.x - 1] +
                     context.horizontalCellPadding +
                     context.cellRightBorder +
                     std::max(context.horizontalSpacing.getSpacingAfter(uint32_t(it.x - 1)),
                              context.horizontalBorderInnerWidth) +
                     context.cellLeftBorder;

      int widgetHeight = it.widget().getHeight();

      bool horizontallyStretchable = it.widget().isHorizontallyStretchable();
      bool verticallyStretchable = it.widget().isVerticallyStretchable();
      if (horizontallyStretchable || verticallyStretchable)
      {
        int targetWidth = horizontallyStretchable ? this->columnWidths[it.x] - it.widget().getHorizontalMargins() : it.widget().getWidth();
        int targetHeight = verticallyStretchable ? this->rowHeights[it.y] - it.widget().getVerticalMargins() : it.widget().getHeight();
        if (targetWidth != it.widget().getWidth() || targetHeight != it.widget().getHeight())
          it.widget().setSize(targetWidth,
                              targetHeight,
                              SetSizeInfo(targetWidth != it.widget().getWidth()? Change::Stretching : Change::Normal,
                                          targetHeight != it.widget().getHeight() ? Change::Stretching : Change::Normal));
      }
      if (it.widget().getWidth() > this->columnWidths[it.x] && it.widget().isHorizontallySquashable())
        it.widget().setSize(this->columnWidths[it.x] - it.widget().getHorizontalMargins(), it.widget().getHeight(), SetSizeInfo(Change::Squashing, Change::Nothing));

      if (widgetHeight != it.widget().getHeight())
        resolveHeights = true;

      int widgetTop = locationY;
      int widgetLeft = locationX;
      // vertical alignment to center
      AreaAlign itemAlignment = this->style.getColumnAlignments()->getItem(it.x);
      HorizontalAlign horizontalAlignment = toHorizontalAlign(itemAlignment);
      VerticalAlign verticalAlignment = toVerticalAlign(itemAlignment);
      switch (horizontalAlignment)
      {
        case HorizontalAlign::Left: widgetLeft += it.widget().getLeftMargin(); break;
        case HorizontalAlign::Center: widgetLeft += (this->columnWidths[it.x] - it.widget().getWidth() - it.widget().getHorizontalMargins()) / 2 + it.widget().getLeftMargin(); break;
        case HorizontalAlign::Right: widgetLeft += this->columnWidths[it.x] - it.widget().getWidth() - it.widget().getRightMargin(); break;
        default: break;
      }

      switch (verticalAlignment)
      {
        case VerticalAlign::Top: if (!this->verticalCentering) { widgetTop += it.widget().getTopMargin(); break; } [[fallthrough]];
        case VerticalAlign::Center: widgetTop += (this->rowHeights[it.y] - it.widget().getHeight() - it.widget().getVerticalMargins()) / 2 + it.widget().getTopMargin(); break;
        case VerticalAlign::Bottom: widgetTop += this->rowHeights[it.y] - it.widget().getHeight() - it.widget().getBottomMargin(); break;
        default: break;
      }
      it.widget().setLocation(widgetLeft, widgetTop);
    }

    if (resolveHeights)
    {
      this->calculateRowHeights(context);

      locationY = context.startingLocationY;
      for (Table::iterator it : TableIterator(*this))
      {
        // next row
        if (it.x == 0 && it.y != 0)
        {
          locationY += this->rowHeights[it.y - 1] +
                       std::max(context.verticalSpacing.getSpacingAfter(uint32_t(it.y - 1)),
                                context.verticalBorderInnerWidth) +
                       context.verticalCellPadding;
        }

        int widgetTop = locationY;
        // vertical alignment to center
        if (this->verticalCentering)
          widgetTop += (this->rowHeights[it.y] - it.widget().getHeight() - it.widget().getVerticalMargins()) / 2 + it.widget().getTopMargin();

        it.widget().setLocation(it.widget().getLocation().x, widgetTop);
      }
    }

    int width = 0;
    for (int columnWidth : this->columnWidths)
      width += columnWidth;
    width += Table::sumSpacingsWithOptionalInnerBorder(context.horizontalSpacing, maxXCount - 1, context.horizontalBorderInnerWidth);
    width += maxXCount * context.horizontalCellExtraSpace;
    if (this->bordersToDraw & LEFT_BORDER)
      width += context.borderWidth;
    if (this->bordersToDraw & RIGHT_BORDER)
      width += context.borderWidth;

    int height = this->calculateContentHeight(context);

    // called to prevent to recursively call this function as reaction to set size
    Widget::setSize(width + this->getHorizontalPaddings(), height + this->getVerticalPaddings());
  }

  int Table::calculateContentHeight(const LayoutingContext& context) const
  {
    int height = 0;
    for (size_t i = 0; i < this->rowHeights.size(); i++)
      height += this->rowHeights[i];
    int borderHeight = this->getBorderWidth();
    height += Table::sumSpacingsWithOptionalInnerBorder(context.verticalSpacing,
                                                        int(this->rowHeights.size()) - 1,
                                                        context.verticalBorderInnerWidth);
    if (this->bordersToDraw & TOP_BORDER)
      height += borderHeight;
    if (this->bordersToDraw & BOTTOM_BORDER)
      height += borderHeight;
    height += int(this->rowHeights.size()) * (context.topCellPadding + context.bottomCellPadding);
    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
    {
      height += columnGraphicalSet->getTopBorder();
      height += columnGraphicalSet->getBottomBorder();
    }
    return height;
  }

  int Table::calculateExpectedHeight(int numRows, int slotHeight) const
  {
    int borderHeight = this->getBorderWidth();
    int height = slotHeight * numRows;
    height += Table::sumSpacingsWithOptionalInnerBorder(this->style.getVerticalSpacing(),
                                                        numRows - 1,
                                                        (this->bordersToDraw & INNER_HORIZONTAL_BORDERS) ? borderHeight : 0);
    height += numRows * (this->style.getTopCellPadding() + this->style.getBottomCellPadding());
    if (this->bordersToDraw & TOP_BORDER)
      height += borderHeight;
    if (this->bordersToDraw & BOTTOM_BORDER)
      height += borderHeight;
    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
    {
      height += columnGraphicalSet->getTopBorder();
      height += columnGraphicalSet->getBottomBorder();
    }
    return height;
  }

  void Table::calculateRowHeights(const LayoutingContext& context)
  {
    std::vector<char> stretchable;
    int rowCount = this->getRowCount();
    stretchable.resize(rowCount, true);
    this->rowHeights.clear();
    this->rowHeights.resize(rowCount);

    for (Table::const_iterator it : ConstTableIterator(*this))
    {
      this->rowHeights[it.y] = std::max(this->rowHeights[it.y], it.widget().getHeight() + it.widget().getVerticalMargins());

      if (int(stretchable.size()) <= it.y)
        stretchable.resize(it.y + 1, true);
      if (!it.widget().isVerticallyStretchable())
        stretchable[it.y] = false;
    }

    int stretchableCount = std::count_if(stretchable.begin(), stretchable.end(), [](char value){ return value != 0; });

    if (context.reactionToSetSize && stretchableCount > 0)
    {
      int height = this->calculateContentHeight(context);
      if (height < this->getContentHeight())
      {
        int extraSpace = (this->getContentHeight() - height) / stretchableCount;
        for (uint32_t i = 0; i < this->rowHeights.size(); ++i)
          if (stretchable[i])
            this->rowHeights[i] += extraSpace;
      }
    }
  }

  std::vector<int> Table::getColumnWidths(int columnCount) const
  {
    if (columnCount == 0)
      return std::vector<int>();

    const TableStyle::Widths* styleWidths = this->style.getColumnWidths();
    std::vector<TableStyle::Widths::Item> columnWidths = styleWidths->getData();
    std::vector<int> result;

    result.resize(columnCount, 0);

    columnWidths.reserve(columnCount);
    for (int i = int(columnWidths.size()); i < columnCount; ++i)
      columnWidths.emplace_back(styleWidths->getItem(i));

    for (int i = 0; i < columnCount; ++i)
      result[i] = columnWidths[i].minimalWidth;

    int usedElementCount = 0;
    for (Table::const_iterator it : ConstTableIterator(*this))
    {
      result[it.x] = std::max(result[it.x], it.widget().getWidth() + it.widget().getHorizontalMargins());
      if (int maxWidth = columnWidths[it.x].maximalWidth)
        result[it.x] = std::min(result[it.x], maxWidth);
      ++usedElementCount;
    }

    if (this->style.getWideAsColumnCount())
      for (int column = usedElementCount; column < columnCount; ++column)
        result[column] = this->style.getColumnWidths()->getItem(column).minimalWidth;

    if (this->unifyWidth)
    {
      int widestWidth = 0;
      for (int columnWidth : result)
        if (widestWidth < columnWidth)
          widestWidth = columnWidth;
      for (int& columnWidth : result)
        columnWidth = widestWidth;
    }
    return result;
  }

  uint32_t Table::getRowCount() const
  {
    uint32_t result = 0;
    for (Table::const_iterator it : ConstTableIterator(*this))
      result = it.y + 1;
    return result;
  }

  void Table::setColumnCount(int columnCount)
  {
    if (columnCount <= 0)
      abort();
    if (this->columnCount == uint32_t(columnCount))
      return;

    this->columnCount = columnCount;
    this->columnWidths.resize(columnCount);
    this->triggerResize();
  }

  void Table::setVerticalCentering(bool value)
  {
    this->verticalCentering = value;
  }

  uint32_t Table::getColumnCount() const
  {
    return this->columnCount;
  }

  bool Table::getVerticalCentering() const
  {
    return this->verticalCentering;
  }

  void Table::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (const ElementImageSet* background = this->style.getBackgroundGraphicalSet())
    {
      paintEvent.graphics()->pushClippingRect(this, this->getSizeRectangle(), true);
      background->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
      paintEvent.graphics()->popClippingRect();
    }

    const int horizontalCellPadding = this->style.getLeftCellPadding() + this->style.getRightCellPadding();
    const int verticalCellPadding = this->style.getTopCellPadding() + this->style.getBottomCellPadding();
    const TableStyle::Spacings& verticalSpacing = this->style.getVerticalSpacing();
    const TableStyle::Spacings& horizontalSpacing = this->style.getHorizontalSpacing();
    const bool applyGraphicalSetPerColumn = this->style.getApplyRowGraphicalSetPerColumn();
    const int selfWidth = this->getWidth();

    const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet();
    const ElementImageSet* defaultRowGraphicalSet = this->style.getDefaultRowGraphicalSet();
    const ElementImageSet* evenRowGraphicalSet = this->style.getEvenRowGraphicalSet();
    const ElementImageSet* oddRowGraphicalSet = this->style.getOddRowGraphicalSet();
    const ElementImageSet* selectedGraphicalSet = this->style.getSelectedGraphicalSet();
    const ElementImageSet* hoveredGraphicalSet = this->style.getHoveredGraphicalSet();
    const ElementImageSet* clickedGraphicalSet = this->style.getClickedGraphicalSet();
    const ElementImageSet* selectedHoveredGraphicalSet = this->style.getSelectedHoveredGraphicalSet();
    const ElementImageSet* selectedClickedGraphicalSet = this->style.getSelectedClickedGraphicalSet();

    const int horizontalBorder = columnGraphicalSet ? columnGraphicalSet->getHorizontalBorder() : 0;

    // Draw header row
    if (this->hasHeaders && defaultRowGraphicalSet && !this->rowHeights.empty())
    {
      const int height = this->rowHeights[0] + verticalCellPadding + verticalSpacing.getSpacingAfter(0);
      const int y = this->getTopPadding();

      if (!applyGraphicalSetPerColumn)
        defaultRowGraphicalSet->base.draw(paintEvent, Rectangle(Point(0, y), Dimension(selfWidth, height)), absolutePosition);
      else
      {
        int horizontalPosition = this->getLeftPadding();
        for (uint32_t column = 0; column < this->columnCount; ++column)
        {
          const int width = this->columnWidths[column] + horizontalBorder + horizontalCellPadding;
          defaultRowGraphicalSet->base.draw(paintEvent, Rectangle(Point(horizontalPosition, y), Dimension(width, height)), absolutePosition);
          horizontalPosition += width + horizontalSpacing.getSpacingAfter(column);
        }
      }
    }

    const uint32_t selectedIndex = this->getRawSelectedIndex();
    const uint32_t hoveredIndex = this->getRawHoveredIndex();
    bool hoverIsClicked = this->getHoverIsClicked();

    if (defaultRowGraphicalSet ||
        evenRowGraphicalSet ||
        oddRowGraphicalSet ||
        (selectedIndex != uint32_t(-1) && selectedGraphicalSet) ||
        (hoveredIndex != uint32_t(-1) && hoveredGraphicalSet))
    {
      Graphics* graphics = paintEvent.graphics();
      const bool usingClipping = !graphics->isClippingRectEmpty();
      const Rectangle clipping = graphics->getClippingRectangle();

      int verticalPosition = this->getTopPadding();

      if (columnGraphicalSet)
        verticalPosition += columnGraphicalSet->getTopBorder();
      if (const BorderImageSet* borderImageSet = this->style.getBorder())
        verticalPosition += borderImageSet->lineWidth;

      int startingIndex = 0;

      if (!this->rowHeights.empty())
      {
        if (this->hasHeaders)
        {
          startingIndex = 1;
          verticalPosition += this->rowHeights[0] + verticalCellPadding;
        }

        const int leftPadding = this->getLeftPadding();

        for (uint32_t i = startingIndex; i < this->rowHeights.size(); ++i)
        {
          const ElementImageSet* graphicalSet = defaultRowGraphicalSet;
          const bool even = i % 2 == 0;
          if (even && evenRowGraphicalSet)
            graphicalSet = evenRowGraphicalSet;
          else if (!even && oddRowGraphicalSet)
            graphicalSet = oddRowGraphicalSet;

          const bool selected = selectedIndex != uint32_t(-1) && selectedIndex == i;
          const bool hovered = hoveredIndex != uint32_t(-1) && hoveredIndex == i;

          if (selected && hovered && hoverIsClicked && selectedClickedGraphicalSet)
            graphicalSet = selectedClickedGraphicalSet;
          else if (selected && hovered && selectedHoveredGraphicalSet)
            graphicalSet = selectedHoveredGraphicalSet;
          else if (hovered && hoverIsClicked && clickedGraphicalSet)
            graphicalSet = clickedGraphicalSet;
          else if (hovered && hoveredGraphicalSet)
            graphicalSet = hoveredGraphicalSet;
          else if (selected && selectedGraphicalSet)
            graphicalSet = selectedGraphicalSet;

          if (graphicalSet != nullptr)
          {
            const int verticalRowSpacing = verticalSpacing.getSpacingAfter(i);
            const int height = this->rowHeights[i] + verticalCellPadding + verticalRowSpacing;
            const int y = verticalPosition + verticalRowSpacing / 2;

            Rectangle rectangle(Point(0, y), Dimension(selfWidth, height));

            if (!usingClipping || clipping.collide(rectangle + absolutePosition))
            {
              if (!applyGraphicalSetPerColumn)
                graphicalSet->base.draw(paintEvent, rectangle, absolutePosition);
              else
              {
                int horizontalPosition = leftPadding;
                for (uint32_t column = 0; column < this->columnCount; ++column)
                {
                  const int width = this->columnWidths[column] + horizontalBorder + horizontalCellPadding;
                  graphicalSet->base.draw(paintEvent, Rectangle(Point(horizontalPosition, y), Dimension(width, height)), absolutePosition);
                  horizontalPosition += width + horizontalSpacing.getSpacingAfter(column);
                }
              }
            }
          }

          verticalPosition += this->rowHeights[i] + verticalCellPadding;
          if (i != 0)
            verticalPosition += verticalSpacing.getSpacingAfter(i - 1);
        }
      }
    }

    // Column graphics after background so they render over the background
    if (columnGraphicalSet)
    {
      const int contentsHeight = this->getContentHeight();

      int horizontalPosition = this->getLeftPadding();
      for (uint32_t i = 0; i < this->columnCount; ++i)
      {
        const int width = this->columnWidths[i] + horizontalBorder + horizontalCellPadding;
        columnGraphicalSet->base.draw(paintEvent, Rectangle(horizontalPosition, 0, width, contentsHeight), absolutePosition);
        horizontalPosition += width + horizontalSpacing.getSpacingAfter(i);
      }
    }

    this->tryToDrawVerticalLines(paintEvent);
    this->tryToDrawHorizontalLines(paintEvent);

    if (this->style.usesBorder())
      this->drawBorder(paintEvent);

    if (paintEvent.graphics()->debugView)
      paintEvent.graphics()->drawRectangle(this->getSizeRectangle(), agui::Color(1, 0, 0));
  }

  int Table::getRowYPosition(uint32_t row) const
  {
    if (this->hasHeaders)
      ++row;

    int y = this->style.getTopPadding();

    int borderHeight = this->getBorderWidth();
    int verticalCellPadding = this->style.getTopCellPadding() + this->style.getBottomCellPadding();
    const TableStyle::Spacings& verticalSpacing = this->style.getVerticalSpacing();

    for (size_t i = 0; i < std::min(size_t(row), this->rowHeights.size()); ++i)
      y += this->rowHeights[i] + std::max(verticalSpacing.getSpacingAfter(uint32_t(i)), borderHeight) + verticalCellPadding;

    return y;
  }

  void Table::unifyColumnWidthsWith(Table* table)
  {
    if (this->connector == nullptr)
    {
      if (table->connector == nullptr)
      {
        this->connector = new TableConnector();
        this->connector->add(this);
        table->connector = this->connector;
        this->connector->add(table);
      }
      else
      {
        table->connector->add(this);
        this->connector = table->connector;
      }
    }
    else
    {
      this->connector->add(table);
      table->connector = this->connector;
    }
  }

  void Table::logic(double)
  {
    if (this->connector)
      this->connector->logic();
  }

  Widget* Table::getWidgetUnderMouse(Point mousePosition, const Point thisAbsolutePosition, Rectangle absolutePositionClip, TransparentValue parentTransparent)
  {
    Point contentAreaAbsolutePosition = thisAbsolutePosition + this->getLeftTopPadding();
    Widget* lastUnderMouse = nullptr;

    if (this->getChildCount() < 50 || this->visibleChildren.empty()) // simple version for small tables
    {
      const Point intersectionPoint = mousePosition - contentAreaAbsolutePosition;
      for (auto childIt = this->rbegin(); childIt != this->rend(); childIt++)
        if ((*childIt)->isUnderMouse(intersectionPoint))
        {
          lastUnderMouse = *childIt;
          break;
        }
    }
    else
      lastUnderMouse = this->getChildUnderMouseWithBinaryRowSearch(mousePosition, contentAreaAbsolutePosition, absolutePositionClip);

    TransparentValue transparent = transparentOpaqueMask(this->isTransparent(), parentTransparent);

    if (lastUnderMouse)
      if (agui::Widget* underMouse = lastUnderMouse->getWidgetUnderMouse(mousePosition,
                                                                         this->getChildRelativePosition(lastUnderMouse) + thisAbsolutePosition,
                                                                         absolutePositionClip & (this->getChildRelativeClippingRectangle(lastUnderMouse) +
                                                                                                 thisAbsolutePosition), transparent))
        return underMouse;

    return this->getUnderMouseWithTransparency(transparent);
  }

  Widget* Table::getChildUnderMouseWithBinaryRowSearch(Point mousePosition, Point thisAbsolutePosition, Rectangle absolutePositionClip)
  {
    // this version filters out non-visible rows and searches the child widgets only within those rows

    const Point intersectionPoint = mousePosition - thisAbsolutePosition;
    uint32_t rowCount = (uint32_t(this->visibleChildren.size()) - 1) / this->getColumnCount() + 1;
    int visibleAreaTop = absolutePositionClip.y - thisAbsolutePosition.y; // start of relevant window, lower y coord
    int visibleAreaBottom = std::min(this->getContentHeight(), absolutePositionClip.y + absolutePositionClip.height - thisAbsolutePosition.y);

    std::vector<int> memoizedRowBottomPixels(rowCount);
    auto calculateRowBottomPixel = [&](uint32_t rowIndex) -> int
    {
      if (memoizedRowBottomPixels[rowIndex] != 0)
        return memoizedRowBottomPixels[rowIndex];

      uint32_t startChildIndex = rowIndex * this->getColumnCount();
      uint32_t endChildIndex = std::min((rowIndex + 1) * this->getColumnCount(), uint32_t(this->visibleChildren.size()));
      int rowBottomPixel = 0;

      for (uint32_t childIndex = startChildIndex; childIndex < endChildIndex; ++childIndex)
      {
        Widget* child = this->visibleChildren[childIndex];
        rowBottomPixel = std::max(rowBottomPixel, child->getLocation().y + child->getHeight());
      }

      memoizedRowBottomPixels[rowIndex] = rowBottomPixel;
      return rowBottomPixel;
    };

    auto calculateRowTopPixel = [&](uint32_t rowIndex) -> int
    {
      if (rowIndex > 0)
        return calculateRowBottomPixel(rowIndex - 1); // row is considered to contain the gap above it. top of row is the bottom of previous row.
      return 0;
    };

    auto findFirstRow = [](uint32_t lower, uint32_t upper, auto predicate) -> uint32_t
    {
      while (lower < upper)
      {
        uint32_t guess = lower + (upper - lower) / 2;
        if (predicate(guess))
          upper = guess;
        else
          lower = guess + 1u;
      }
      return lower;
    };

    const uint32_t firstVisibleRow = findFirstRow(0, rowCount, [&](uint32_t row) { return calculateRowBottomPixel(row) >= visibleAreaTop; });
    const uint32_t lastVisibleRow = findFirstRow(firstVisibleRow, rowCount, [&](uint32_t row) { return calculateRowTopPixel(row) > visibleAreaBottom; });

    const uint32_t start = firstVisibleRow * this->getColumnCount();
    const uint32_t end = std::min(lastVisibleRow * this->getColumnCount(), uint32_t(this->visibleChildren.size()));
    for (uint32_t childIndex = end; childIndex > start; childIndex--)
    {
      Widget* child = this->visibleChildren[childIndex - 1u];
      if (child->isUnderMouse(intersectionPoint))
        return child;
    }

    return nullptr;
  }

  void Table::clear()
  {
    super::clear();
    this->visibleChildren.clear();
  }

  void Table::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    super::remove(widget, keepWidgetAlive);
    this->visibleChildren.clear();
  }

  Table& Table::alignColumn(int columnIndex, agui::AreaAlign align)
  {
    this->style.setColumnAlignments(this->style.getColumnAlignments()->copy().setItem(columnIndex, align));
    return *this;
  }

  TransparentValue Table::isTransparent() const
  {
    if ((this->style.getBackgroundGraphicalSet() && this->style.getBackgroundGraphicalSet()->base.type != ElementImageSet::Layer::Type::None) ||
        (this->style.getColumnGraphicalSet() && this->style.getColumnGraphicalSet()->base.type != ElementImageSet::Layer::Type::None) ||
        (this->style.getEvenRowGraphicalSet() && this->style.getEvenRowGraphicalSet()->base.type != ElementImageSet::Layer::Type::None) ||
        (this->style.getOddRowGraphicalSet() && this->style.getOddRowGraphicalSet()->base.type != ElementImageSet::Layer::Type::None))
      return TransparentValue::No;
    return TransparentValue::DependsOnChildren;
  }

  std::vector<int> Table::getVerticalBorderPositions(int borderWidth) const
  {
    // borders are drawn over horizontal spacings.
    // Order of stuff is:
    // [columnGraphicalSet.leftBorder] - leftPadding - (cell leftPadding - row[i] width - cell rightPadding) - horizontal spacing - (next row)

    if (this->columnWidths.empty())
      return std::vector<int>();
    std::vector<int> result;
    result.reserve(this->columnWidths.size());

    int horizontalCellPadding = this->style.getLeftCellPadding() + this->style.getRightCellPadding();
    const TableStyle::Spacings& horizontalSpacing = this->style.getHorizontalSpacing();
    int horizontalPosition = this->getLeftPadding();
    if (this->bordersToDraw & Table::LEFT_BORDER)
      horizontalPosition += borderWidth;

    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      horizontalPosition += columnGraphicalSet->getLeftBorder();

    assert(this->columnWidths.size() == this->columnCount);
    for (uint32_t i = 0; i < this->columnCount - 1; ++i)
    {
      const int horizontalColumnSpacing = horizontalSpacing.getSpacingAfter(i);
      horizontalPosition += this->columnWidths[i] + horizontalCellPadding;
      if (horizontalColumnSpacing > borderWidth)
        horizontalPosition += (horizontalColumnSpacing - borderWidth) / 2;
      result.push_back(horizontalPosition);
      if (horizontalColumnSpacing > borderWidth)
        horizontalPosition += horizontalColumnSpacing - borderWidth - (horizontalColumnSpacing - borderWidth) / 2;
      else
        horizontalPosition += std::max(horizontalColumnSpacing, borderWidth);
    }
    return result;
  }

  std::vector<int> Table::getHorizontalBorderPositions(int borderWidth, bool borderOnTop) const
  {
    // borders are drawn over vertical spacings.
    // Order of stuff is:
    // [columnGraphicalSet.topBorder] - topPadding - (cell topPadding - row[i] height - cell bottomPadding) - vertical spacing - (next row)

    if (this->rowHeights.empty())
      return std::vector<int>();
    std::vector<int> result;
    result.reserve(this->rowHeights.size());

    int topCellPadding = this->style.getTopCellPadding();
    int bottomCellPadding = this->style.getBottomCellPadding();
    const TableStyle::Spacings& verticalSpacing = this->style.getVerticalSpacing();
    int verticalPosition = this->getTopPadding();
    if (this->bordersToDraw & Table::TOP_BORDER)
      verticalPosition += topCellPadding;

    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      verticalPosition += columnGraphicalSet->getTopBorder();
    if (borderOnTop)
      verticalPosition += borderWidth;

    for (uint32_t i = 0; i < this->rowHeights.size() - 1; ++i)
    {
      const int verticalRowSpacing = verticalSpacing.getSpacingAfter(i);
      const int extraVerticalRowSpacingHalf = (verticalRowSpacing - borderWidth) / 2;
      verticalPosition += this->rowHeights[i] + bottomCellPadding;
      if (extraVerticalRowSpacingHalf > 0)
        verticalPosition += extraVerticalRowSpacingHalf;
      result.push_back(verticalPosition);
      verticalPosition += topCellPadding + borderWidth;
      if (verticalRowSpacing > borderWidth)
        verticalPosition += (verticalRowSpacing - borderWidth - extraVerticalRowSpacingHalf);
    }
    return result;
  }

  void Table::tryToDrawVerticalLines(const PaintEvent& paintEvent)
  {
    if (!this->drawVerticalLines)
      return;

    std::vector<int> borderPositions = getVerticalBorderPositions(1);
    const Color color = this->style.getVerticalLineColor();
    for (auto horizontalPosition : borderPositions)
      paintEvent.graphics()->drawLine(Point(horizontalPosition, 0),
                                      Point(horizontalPosition, this->getHeight()),
                                      color);
  }

  void Table::tryToDrawHorizontalLines(const PaintEvent& paintEvent)
  {
    if (!this->drawHorizontalLines &&
        !this->drawHorizontalLineAfterHeaders)
      return;

    std::vector<int> borderPositions = getHorizontalBorderPositions(1, false /* dont drawing line at the top */);
    const Color color = this->style.getHorizontalLineColor();
    if (!borderPositions.empty())
      for (uint32_t i = 0; i < (this->drawHorizontalLines ? borderPositions.size() : 1); i++)
        paintEvent.graphics()->drawLine(Point(0, borderPositions[i]),
                                        Point(this->getWidth(), borderPositions[i]),
                                        color);
  }

  void Table::drawBorder(const PaintEvent& paintEvent)
  {
    // these are done separately as they are common cases that are more efficient
    if (this->bordersToDraw == FULL_GRID_BORDER)
    {
      this->drawBorderGrid(paintEvent);
      return;
    }
    if (this->bordersToDraw == OUTER_BORDER)
    {
      this->drawOuterBorder(paintEvent);
      return;
    }

    // inner lines
    if (this->bordersToDraw & INNER_HORIZONTAL_BORDERS)
      this->drawInnerHorizontalBorders(paintEvent);
    if (this->bordersToDraw & INNER_VERTICAL_BORDERS)
      this->drawInnerVerticalBorders(paintEvent);
    if ((this->bordersToDraw & INNER_GRID_BORDER) == INNER_GRID_BORDER)
      this->drawInnerCrosses(paintEvent);

    // outer borders
    if (this->bordersToDraw & TOP_BORDER)
      this->drawOuterBorderTop(paintEvent);
    if (this->bordersToDraw & RIGHT_BORDER)
      this->drawOuterBorderRight(paintEvent);
    if (this->bordersToDraw & BOTTOM_BORDER)
      this->drawOuterBorderBottom(paintEvent);
    if (this->bordersToDraw & LEFT_BORDER)
      this->drawOuterBorderLeft(paintEvent);

    // corners
    if ((this->bordersToDraw & (TOP_BORDER | RIGHT_BORDER)) == (TOP_BORDER | RIGHT_BORDER))
      this->drawOuterBorderTopRightCorner(paintEvent);
    if ((this->bordersToDraw & (RIGHT_BORDER | BOTTOM_BORDER)) == (RIGHT_BORDER | BOTTOM_BORDER))
      this->drawOuterBorderBottomRightCorner(paintEvent);
    if ((this->bordersToDraw & (BOTTOM_BORDER | LEFT_BORDER)) == (BOTTOM_BORDER | LEFT_BORDER))
      this->drawOuterBorderBottomLeftCorner(paintEvent);
    if ((this->bordersToDraw & (LEFT_BORDER | TOP_BORDER)) == (LEFT_BORDER | TOP_BORDER))
      this->drawOuterBorderTopLeftCorner(paintEvent);

    // connecting borders to inner lines
    if ((this->bordersToDraw & (INNER_HORIZONTAL_BORDERS | RIGHT_BORDER)) == (INNER_HORIZONTAL_BORDERS | RIGHT_BORDER))
      this->drawOuterBorderRightTs(paintEvent);
    if ((this->bordersToDraw & (INNER_HORIZONTAL_BORDERS | LEFT_BORDER)) == (INNER_HORIZONTAL_BORDERS | LEFT_BORDER))
      this->drawOuterBorderLeftTs(paintEvent);
    if ((this->bordersToDraw & (INNER_VERTICAL_BORDERS | TOP_BORDER)) == (INNER_VERTICAL_BORDERS | TOP_BORDER))
      this->drawOuterBorderTopTs(paintEvent);
    if ((this->bordersToDraw & (INNER_VERTICAL_BORDERS | BOTTOM_BORDER)) == (INNER_VERTICAL_BORDERS | BOTTOM_BORDER))
      this->drawOuterBorderBottomTs(paintEvent);

    // border under first row
    if (this->bordersToDraw & UNDER_FIRST_ROW_BORDER)
    {
      assert((this->bordersToDraw & INNER_VERTICAL_BORDERS) == 0); // not supported atm
      this->drawInnerHorizontalFirstBorder(paintEvent);
    }
  }

  void Table::drawBorderGrid(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->drawInnerGrid(paintEvent, false);
    this->drawInnerCrosses(paintEvent);
    this->drawOuterBorder(paintEvent);
    this->drawOuterBorderTs(paintEvent);
  }

  void Table::drawOuterBorder(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    if (this->getSize().width == 0 && this->getSize().height == 0)
      return;
    this->style.getBorder()->drawBorder(paintEvent, Point(0, 0), Dimension(this->getWidth(), this->getHeight()));
  }

  void Table::drawOuterBorderTs(const PaintEvent& paintEvent)
  {
    this->drawOuterBorderTopTs(paintEvent);
    this->drawOuterBorderRightTs(paintEvent);
    this->drawOuterBorderBottomTs(paintEvent);
    this->drawOuterBorderLeftTs(paintEvent);
  }

  void Table::drawOuterBorderTopTs(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> borderPositions = getVerticalBorderPositions(this->style.getBorder()->lineWidth);
    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawTerminal(paintEvent, Point(verticalPosition, 0), BorderImageSet::Terminal::TopT);
  }

  void Table::drawOuterBorderTop(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawHorizontalLine(paintEvent, Point(0, 0), this->getWidth());
  }

  void Table::drawOuterBorderTopRightCorner(const PaintEvent & paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawTerminal(paintEvent,
                                          Point(this->getWidth() - this->style.getBorder()->lineWidth, 0),
                                          BorderImageSet::Terminal::TopRightCorner);
  }

  void Table::drawOuterBorderRightTs(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> borderPositions = getHorizontalBorderPositions(this->style.getBorder()->lineWidth, true /* Drawing border on top */);
    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawTerminal(paintEvent,
                                            Point(this->getWidth() - this->style.getBorder()->lineWidth, verticalPosition),
                                            BorderImageSet::Terminal::RightT);
  }

  void Table::drawOuterBorderRight(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawVerticalLine(paintEvent, Point(this->getWidth() - this->style.getBorder()->lineWidth, 0), this->getHeight());
  }

  void Table::drawOuterBorderBottomRightCorner(const PaintEvent & paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawTerminal(paintEvent,
                                          Point(this->getWidth() - this->style.getBorder()->lineWidth,
                                                this->getHeight() - this->style.getBorder()->lineWidth),
                                          BorderImageSet::Terminal::BottomRightCorner);
  }

  void Table::drawOuterBorderBottomTs(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> borderPositions = getVerticalBorderPositions(this->style.getBorder()->lineWidth);
    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawTerminal(paintEvent,
                                            Point(verticalPosition, this->getHeight() - this->style.getBorder()->lineWidth),
                                            BorderImageSet::Terminal::BottomT);
  }

  void Table::drawOuterBorderBottom(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawHorizontalLine(paintEvent, Point(0, this->getHeight() - this->style.getBorder()->lineWidth), this->getWidth());
  }

  void Table::drawOuterBorderBottomLeftCorner(const PaintEvent & paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawTerminal(paintEvent,
                                          Point(0, this->getHeight() - this->style.getBorder()->lineWidth),
                                          BorderImageSet::Terminal::BottomLeftCorner);
  }

  void Table::drawOuterBorderLeftTs(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> borderPositions = getHorizontalBorderPositions(this->style.getBorder()->lineWidth, true /* Drawing border on top */);
    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawTerminal(paintEvent, Point(0, verticalPosition), BorderImageSet::Terminal::LeftT);
  }

  void Table::drawOuterBorderLeft(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawVerticalLine(paintEvent, Point(0, 0), this->getHeight());
  }

  void Table::drawOuterBorderTopLeftCorner(const PaintEvent & paintEvent)
  {
    if (!this->style.usesBorder())
      return;
    this->style.getBorder()->drawTerminal(paintEvent, Point(0, 0), BorderImageSet::Terminal::TopLeftCorner);
  }

  void Table::drawInnerGrid(const PaintEvent& paintEvent, bool withEnds)
  {
    if (!this->style.usesBorder())
      return;
    this->drawInnerHorizontalBorders(paintEvent, withEnds);
    this->drawInnerVerticalBorders(paintEvent, withEnds);
  }

  void Table::drawInnerHorizontalBorders(const PaintEvent& paintEvent, bool withEnds)
  {
    if (!this->style.usesBorder())
      return;
    std::vector<int> borderPositions = getHorizontalBorderPositions(this->style.getBorder()->lineWidth, this->bordersToDraw & TOP_BORDER);

    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawHorizontalLine(paintEvent,
                                                  Point(0, verticalPosition),
                                                  this->getWidth(),
                                                  withEnds ? BorderImageSet::Terminal::LeftEnd : BorderImageSet::Terminal::Empty,
                                                  withEnds ? BorderImageSet::Terminal::RightEnd : BorderImageSet::Terminal::Empty);
  }

  void Table::drawInnerVerticalBorders(const PaintEvent& paintEvent, bool withEnds)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> borderPositions = getVerticalBorderPositions(this->style.getBorder()->lineWidth);

    for (auto verticalPosition : borderPositions)
      this->style.getBorder()->drawVerticalLine(paintEvent,
                                                Point(verticalPosition, 0),
                                                this->getHeight(),
                                                withEnds ? BorderImageSet::Terminal::TopEnd : BorderImageSet::Terminal::Empty,
                                                withEnds ? BorderImageSet::Terminal::BottomEnd : BorderImageSet::Terminal::Empty);
  }

  void Table::drawInnerCrosses(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder())
      return;

    std::vector<int> horizontalBorderPositions = getHorizontalBorderPositions(this->style.getBorder()->lineWidth, true /* Drawing border on top */);
    std::vector<int> verticalBorderPositions = getVerticalBorderPositions(this->style.getBorder()->lineWidth);

    for (auto horizontalPosition : horizontalBorderPositions)
      for (auto verticalPosition : verticalBorderPositions)
        this->style.getBorder()->drawTerminal(paintEvent, Point(verticalPosition, horizontalPosition), BorderImageSet::Terminal::Cross);
  }

  void Table::drawInnerHorizontalFirstBorder(const PaintEvent& paintEvent)
  {
    if (!this->style.usesBorder() || this->getRowCount() < 2)
      return;

    int firstRowHeight = this->rowHeights[0];
    int verticalPosition = this->getTopPadding() + this->style.getTopCellPadding() + this->style.getBottomCellPadding() + firstRowHeight;
    if (const ElementImageSet* columnGraphicalSet = this->style.getColumnGraphicalSet())
      verticalPosition += columnGraphicalSet->getTopBorder();

    this->style.getBorder()->drawHorizontalLine(paintEvent,
                                                Point(0, verticalPosition),
                                                this->getWidth(),
                                                this->bordersToDraw & LEFT_BORDER ? BorderImageSet::Terminal::LeftEnd : BorderImageSet::Terminal::None,
                                                this->bordersToDraw & RIGHT_BORDER ? BorderImageSet::Terminal::RightEnd : BorderImageSet::Terminal::None);

    if (this->bordersToDraw & LEFT_BORDER)
      this->style.getBorder()->drawTerminal(paintEvent, Point(0, verticalPosition), BorderImageSet::Terminal::LeftT);
    if (this->bordersToDraw & RIGHT_BORDER)
      this->style.getBorder()->drawTerminal(paintEvent,
                                            Point(this->getWidth() - this->style.getBorder()->lineWidth, verticalPosition),
                                            BorderImageSet::Terminal::RightT);

  }

  void Table::stretchColumnWidthsToSize(const LayoutingContext& context)
  {
    int futureWidth = 0;
    for (uint32_t i = 0; i < std::min(uint32_t(this->columnWidths.size()), context.childrenCount); ++i)
    {
      if (i != 0)
        futureWidth += std::max(context.horizontalSpacing.getSpacingAfter(i - 1), context.horizontalBorderInnerWidth);
      futureWidth += this->columnWidths[i] + context.horizontalCellExtraSpace;
    }
    futureWidth += this->getHorizontalPaddings();
    if (this->bordersToDraw & LEFT_BORDER)
      futureWidth += context.borderWidth;
    if (this->bordersToDraw & RIGHT_BORDER)
      futureWidth += context.borderWidth;
    bool sizeExtendedByDisallowingSizeDecrease = !this->canDecreaseWidth() && futureWidth < this->getWidth();

    // leave stretching to be the old way, but squashing will be done by the SquashCalculatorAlready
    if (context.reactionToSetSize && this->getWidth() > futureWidth ||
        !context.reactionToSetSize && this->style.getMinimalWidth() > futureWidth ||
        sizeExtendedByDisallowingSizeDecrease)
    {
      int wantedSize = (context.reactionToSetSize || sizeExtendedByDisallowingSizeDecrease) ? this->getWidth() : this->style.getMinimalWidth();
      int32_t widthDiff = wantedSize - futureWidth;
      auto stretchableColumns = std::make_unique<bool[]>(this->columnWidths.size());

      // for stretching, just one of the cells in the column needs to be stretchable to make the whole column stretchable
      for (uint32_t i = 0; i < this->columnWidths.size(); ++i)
        stretchableColumns[i] = false;

      for (Table::iterator it : TableIterator(*this))
        if (it.widget().isHorizontallyStretchable())
          stretchableColumns[it.x] = true;
      int32_t stretchableColumnCount = 0;
      for (uint32_t i = 0; i < this->columnWidths.size(); ++i)
        if (stretchableColumns[i])
          ++stretchableColumnCount;
      if (stretchableColumnCount > 0)
      {
        int32_t stretchPerColumn = widthDiff / stretchableColumnCount;
        for (uint32_t i = 0; i < this->columnWidths.size(); ++i)
          if (stretchableColumns[i])
            this->columnWidths[i] += stretchPerColumn;
      }
    }
    // squashing
    else if (context.reactionToSetSize && this->getWidth() < futureWidth ||
             !context.reactionToSetSize && this->style.getMaximalWidth() != 0 && this->style.getMaximalWidth() < futureWidth)
    {
      SquashCalculator calculator(GuiDirection::Horizontal);
      int neededSize = this->getContentWidth();
      const TableStyle::Widths& styleWidths = *this->style.getColumnWidths();

      for (Table::iterator it : TableIterator(*this))
      {
        if (it.y == 0)
        {
          if (it.x != 0)
            neededSize -= std::max(context.horizontalSpacing.getSpacingAfter(uint32_t(it.x) - 1), context.horizontalBorderInnerWidth);
          neededSize -= context.horizontalCellExtraSpace;
          calculator.add(it.widget());
          if (int styleColumnMinimalWidth = styleWidths.getItem(it.x).minimalWidth)
            calculator.items.back().ensureMinimum(styleColumnMinimalWidth);
        }
        else
          calculator.merge(it.x, it.widget());
      }
      neededSize -= this->getHorizontalPaddings();
      if (this->bordersToDraw & LEFT_BORDER)
        neededSize -= context.borderWidth;
      if (this->bordersToDraw & RIGHT_BORDER)
        neededSize -= context.borderWidth;
      calculator.calculate(neededSize);
      for (uint32_t i = 0; i < calculator.items.size(); ++i)
        this->columnWidths[i] = calculator.items[i].resultWithMarginSize;
    }
  }

  int Table::getBorderWidth() const
  {
    if (const BorderImageSet* borderSet = this->style.getBorder())
     return borderSet->lineWidth;
    return 0;
  }

  Table& Table::operator<<(const HorizontalPusher&)
  {
    EmptyWidget* pusher = new EmptyWidget();
    pusher->style.setHorizontallyStretchable();
    this->add(pusher);
    pusher->autoDestructWhenRemovedFromParent();
    return *this;
  }

  int Table::maximumVerticalSquashSize() const
  {
    if (this->style.isVerticallySquashable() == StretchRule::Off)
      return 0;

    int result = 0;
    int currentRowMaximum = 0;
    uint32_t currentRowIndex = 0;
    for (Table::const_iterator it : ConstTableIterator(*this))
    {
      if (it.x == 0 && it.y != 0)
        result += currentRowIndex < this->rowHeights.size() ? (this->rowHeights[currentRowIndex] - currentRowMaximum) : 0;
      currentRowMaximum = std::max(currentRowMaximum, it.widget().getHeight() - it.widget().maximumVerticalSquashSize() + it.widget().getVerticalMargins());
      currentRowIndex = it.y;
    }
    result += currentRowIndex < this->rowHeights.size() ? (this->rowHeights[currentRowIndex] - currentRowMaximum) : 0;
    return result;
  }

  int Table::maximumHorizontalSquashSize() const
  {
    if (this->style.isHorizontallySquashable() == StretchRule::Off)
      return 0;

    std::vector<int> maximalSizes;
    maximalSizes.resize(this->columnWidths.size());

    for (Table::const_iterator it : ConstTableIterator(*this))
      maximalSizes[it.x] = std::max(maximalSizes[it.x], it.widget().getWidth() - it.widget().maximumHorizontalSquashSize() + it.widget().getHorizontalMargins());

    int result = 0;
    const TableStyle::Widths* columnWidths = this->style.getColumnWidths();
    for (uint32_t i = 0; i < this->columnWidths.size(); ++i)
    {
      int columnMinimalWidth = columnWidths->getItem(i).minimalWidth;
      if (columnMinimalWidth != 0 && columnMinimalWidth > maximalSizes[i])
        maximalSizes[i] = columnMinimalWidth;
      result += std::max(0, this->columnWidths[i] - maximalSizes[i]);
    }
    return result;
  }

  void Table::updateUsesBorder()
  {
    if (this->style.usesBorder())
      this->bordersToDraw = Table::FULL_GRID_BORDER;
    else
      this->bordersToDraw = Table::NO_BORDER;
  }

  agui::Table& Table::setUnifyWidth(bool unifyWidth)
  {
    this->unifyWidth = unifyWidth;
    return *this;
  }

  Table::LayoutingContext::LayoutingContext(ReactionToSetSize reactionToSetSize, const Table& table)
    : reactionToSetSize(reactionToSetSize)
  {
    for (const Widget* widget : table)
      if (widget->isVisible())
        ++this->childrenCount;

    if (this->childrenCount == 0)
      return;

    this->topCellPadding = table.style.getTopCellPadding();
    this->rightCellPadding = table.style.getRightCellPadding();
    this->bottomCellPadding = table.style.getBottomCellPadding();
    this->leftCellPadding = table.style.getLeftCellPadding();
    this->horizontalCellPadding = this->leftCellPadding + this->rightCellPadding;
    this->verticalCellPadding = this->topCellPadding + this->bottomCellPadding;
    this->startingLocationX = this->leftCellPadding;
    this->startingLocationY = this->topCellPadding;
    this->borderWidth = table.getBorderWidth();

    if (const agui::ElementImageSet* columnGraphicalSet = table.style.getColumnGraphicalSet())
    {
      this->cellLeftBorder = columnGraphicalSet->getLeftBorder();
      this->cellRightBorder = columnGraphicalSet->getRightBorder();
      this->cellTopBorder = columnGraphicalSet->getTopBorder();
      this->cellBottomBorder = columnGraphicalSet->getBottomBorder();
      this->startingLocationY += this->cellTopBorder;
      this->startingLocationX += this->cellLeftBorder;
    }

    if (table.bordersToDraw & Table::TOP_BORDER)
      this->startingLocationY += this->borderWidth;
    if (table.bordersToDraw & Table::LEFT_BORDER)
      this->startingLocationX += this->borderWidth;
    this->horizontalSpacing = table.style.getHorizontalSpacing();
    this->verticalSpacing = table.style.getVerticalSpacing();
    this->horizontalBorderInnerWidth = (table.bordersToDraw & INNER_HORIZONTAL_BORDERS) ? this->borderWidth : 0;
    this->verticalBorderInnerWidth = (table.bordersToDraw & INNER_VERTICAL_BORDERS) ? this->borderWidth : 0;

    this->horizontalCellExtraSpace = this->cellLeftBorder + this->cellRightBorder + this->horizontalCellPadding;
    this->verticalCellExtraSpace = this->cellTopBorder + this->cellBottomBorder + this->verticalCellPadding;
  }
}

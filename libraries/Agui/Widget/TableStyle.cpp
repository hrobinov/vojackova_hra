#include "Agui/Widget/Table.hpp"
#include "Agui/Widget/TableStyle.hpp"
#include "Agui/Widget.hpp"
#include <cassert>

namespace agui
{
  TableStyle::TableStyle(const TableStyle* parent)
    : TableStyle(nullptr, parent)
  {}

  TableStyle::TableStyle(Table* relatedWidget, const TableStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void TableStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->horizontalSpacing)
      this->addStyleComment(result, "horizontal_spacing", this->horizontalSpacing->str(), this->propertyStatus(comparedWith, &TableStyle::horizontalSpacing));
    if (this->verticalSpacing)
      this->addStyleComment(result, "vertical_spacing", this->verticalSpacing->str(), this->propertyStatus(comparedWith, &TableStyle::verticalSpacing));
    if (this->topCellPadding &&
        this->leftCellPadding &&
        this->bottomCellPadding &&
        *this->topCellPadding == *this->leftCellPadding &&
        *this->topCellPadding == *this->bottomCellPadding &&
        *this->topCellPadding == *this->leftCellPadding)
    {
      if (this->getParent() || *this->topCellPadding != 0) // don't show the non-affecting sizes in parent
        this->addStyleComment(result, "cell_padding",
                              std::to_string(*this->topCellPadding),
                              this->propertyStatus(comparedWith, &TableStyle::topCellPadding) ||
                              this->propertyStatus(comparedWith, &TableStyle::rightCellPadding) ||
                              this->propertyStatus(comparedWith, &TableStyle::bottomCellPadding) ||
                              this->propertyStatus(comparedWith, &TableStyle::leftCellPadding));
    }
    else
    {
      if (this->topCellPadding)
        this->addStyleComment(result, "top_cell_padding", std::to_string(*this->topCellPadding), this->propertyStatus(comparedWith, &TableStyle::topCellPadding));
      if (this->rightCellPadding)
        this->addStyleComment(result, "right_cell_padding", std::to_string(*this->rightCellPadding), this->propertyStatus(comparedWith, &TableStyle::rightCellPadding));
      if (this->bottomCellPadding)
        this->addStyleComment(result, "bottom_cell_padding", std::to_string(*this->bottomCellPadding), this->propertyStatus(comparedWith, &TableStyle::bottomCellPadding));
      if (this->leftCellPadding)
        this->addStyleComment(result, "left_cell_padding", std::to_string(*this->leftCellPadding), this->propertyStatus(comparedWith, &TableStyle::leftCellPadding));
    }
    if (this->columnAlignments)
      this->addStyleComment(result, "column_alignments", this->columnAlignments->str(), this->propertyStatus(comparedWith, &TableStyle::columnAlignments));
    if (this->columnWidths)
      this->addStyleComment(result, "column_widths", this->columnWidths->str(), this->propertyStatus(comparedWith, &TableStyle::columnWidths));
    if (this->hoveredRowColor)
      this->addStyleComment(result, "hovered_row_color", this->hoveredRowColor->str(), this->propertyStatus(comparedWith, &TableStyle::hoveredRowColor));
    if (this->selectedRowColor)
      this->addStyleComment(result, "selected_row_color", this->selectedRowColor->str(), this->propertyStatus(comparedWith, &TableStyle::selectedRowColor));
    if (this->columnGraphicalSet && this->parent)
      this->addStyleComment(result, "column_graphical_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::columnGraphicalSet));
    if (this->defaultRowGraphicalSet && this->parent)
      this->addStyleComment(result, "default_row_graphical_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::defaultRowGraphicalSet));
    if (this->evenRowGraphicalSet && this->parent)
      this->addStyleComment(result, "even_row_graphical_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::evenRowGraphicalSet));
    if (this->oddRowGraphicalSet && this->parent)
      this->addStyleComment(result, "odd_row_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::oddRowGraphicalSet));
    if (this->hoveredGraphicalSet && this->parent)
      this->addStyleComment(result, "hovered_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::hoveredGraphicalSet));
    if (this->clickedGraphicalSet && this->parent)
      this->addStyleComment(result, "clicked_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::clickedGraphicalSet));
    if (this->selectedGraphicalSet && this->parent)
      this->addStyleComment(result, "selected_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::selectedGraphicalSet));
    if (this->selectedHoveredGraphicalSet && this->parent)
      this->addStyleComment(result, "selected_hovered_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::selectedHoveredGraphicalSet));
    if (this->selectedClickedGraphicalSet && this->parent)
      this->addStyleComment(result, "selected_clicked_graphcial_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::selectedClickedGraphicalSet));
    if (this->borderImageSet && this->parent)
      this->addStyleComment(result, "border_image_set", "redefined", this->propertyStatus(comparedWith, &TableStyle::borderImageSet));
    if (this->wideAsColumnCount && this->parent)
      this->addStyleComment(result, "wide_as_column_count", std::to_string(*this->wideAsColumnCount), this->propertyStatus(comparedWith, &TableStyle::wideAsColumnCount));
  }

  void TableStyle::clear()
  {
    super::clear();
    this->horizontalSpacing.reset();
    this->verticalSpacing.reset();
    this->topCellPadding.reset();
    this->rightCellPadding.reset();
    this->bottomCellPadding.reset();
    this->leftCellPadding.reset();
    this->applyRowGraphicalSetPerColumn.reset();
    this->wideAsColumnCount.reset();
    this->columnGraphicalSet.reset();
    this->defaultRowGraphicalSet.reset();
    this->evenRowGraphicalSet.reset();
    this->oddRowGraphicalSet.reset();
    this->hoveredGraphicalSet.reset();
    this->clickedGraphicalSet.reset();
    this->selectedGraphicalSet.reset();
    this->selectedHoveredGraphicalSet.reset();
    this->selectedClickedGraphicalSet.reset();
    this->rules.reset();
    this->backgroundGraphicalSet.reset();
    this->columnAlignments.reset();
    this->columnWidths.reset();
    this->hoveredRowColor.reset();
    this->selectedRowColor.reset();
    this->verticalLineColor.reset();
    this->horizontalLineColor.reset();
    this->columnOrderingAscendingButtonStyle.reset();
    this->columnOrderingDescendingButtonStyle.reset();
    this->inactiveColumnOrderingAscendingButtonStyle.reset();
    this->inactiveColumnOrderingDescendingButtonStyle.reset();
    this->borderImageSet.reset();
  }

  agui::AreaAlign TableStyle::AreaAlignments::getItem(uint32_t columnIndex) const
  {
    if (this->data.size() <= columnIndex)
      return agui::AreaAlign::LeftTop;
    return this->data[columnIndex];
  }

  TableStyle::AreaAlignments TableStyle::AreaAlignments::copy() const
  {
    return *this;
  }

  TableStyle::AreaAlignments& TableStyle::AreaAlignments::setItem(uint32_t columnIndex, agui::AreaAlign value)
  {
    if (this->data.size() <= columnIndex)
      this->data.resize(columnIndex + 1, agui::AreaAlign::LeftTop);
    this->data[columnIndex] = value;
    return *this;
  }

  TableStyle::AreaAlignments& TableStyle::AreaAlignments::setItems(uint32_t startColumnIndex,
                                                                   uint32_t endColumnIndex,
                                                                   agui::AreaAlign value)
  {
    assert(startColumnIndex <= endColumnIndex);

    if (this->data.size() <= endColumnIndex)
      this->data.resize(endColumnIndex + 1, agui::AreaAlign::LeftTop);

    for (uint32_t i = startColumnIndex; i < endColumnIndex; ++i)
      this->data[i] = value;

    return *this;
  }

  std::string TableStyle::AreaAlignments::str() const
  {
    std::string result = "{";
    for (const agui::AreaAlign& areaAlign : this->data)
    {
      if (&areaAlign != &*this->data.begin())
        result += ", ";
      result += areaAlignemntEnumToString(areaAlign);
    }
    result += "}";
    return result;
  }

  TableStyle::Widths::Item TableStyle::Widths::getItem(uint32_t columnIndex) const
  {
    if (this->unifiedWidth)
      return *unifiedWidth;
    if (this->data.size() <= columnIndex)
      return TableStyle::Widths::Item();
    return this->data[columnIndex];
  }

  TableStyle::Widths TableStyle::Widths::copy() const
  {
    return *this;
  }

  void TableStyle::Widths::setUnifiedColumnWidth(Item item)
  {
    this->unifiedWidth = item;
  }

  TableStyle::Widths& TableStyle::Widths::setItem(uint32_t columnIndex, Item item)
  {
     if (this->data.size() <= columnIndex)
      this->data.resize(columnIndex + 1);
     this->data[columnIndex] = item;
     return *this;
  }

  std::string TableStyle::Widths::str() const
  {
    if (this->unifiedWidth)
      return this->unifiedWidth->str();

    std::string result = "{";
    for (const Item& item : this->data)
    {
      if (&item != &*this->data.begin())
        result += ", ";
      result += item.str();
    }
    result += "}";
    return result;
  }

  std::string TableStyle::Widths::Item::str() const
  {
    std::string result = "[";
    if (this->minimalWidth != 0 && this->maximalWidth == this->minimalWidth)
      result += std::to_string(this->minimalWidth);
    else
    {
      if (this->minimalWidth != 0)
      {
        result += "minimal = ";
        result += std::to_string(this->minimalWidth);
      }
      if (this->maximalWidth != 0)
      {
        if (this->maximalWidth != 0)
          result += " ";
        result += "maximal = ";
        result += std::to_string(this->maximalWidth);
      }
    }
    result += "]";
    return result;
  }

  int TableStyle::Spacings::getSpacingAfter(uint32_t index) const
  {
    if (this->uniformSpacing)
      return *this->uniformSpacing;
    if (index < this->variableSpacings.size())
      return this->variableSpacings[index];
    return 0;
  }

  int TableStyle::Spacings::sumSpacingsBetween(uint32_t indexFrom, uint32_t indexTo) const
  {
    if (indexFrom > indexTo)
      std::swap(indexFrom, indexTo);
    if (this->uniformSpacing)
      return *this->uniformSpacing * (indexTo - indexFrom);

    int32_t sum = 0;
    for (uint32_t i = indexFrom; i < indexTo && i < this->variableSpacings.size(); ++i)
      sum += this->variableSpacings[i];
    return sum;
  }

  std::string TableStyle::Spacings::str() const
  {
    if (this->uniformSpacing)
      return std::to_string(*this->uniformSpacing);

    std::string result = "{";
    for (uint32_t i = 0; i < this->variableSpacings.size(); ++i)
    {
      if (i > 0)
        result += ", ";
      result += std::to_string(this->variableSpacings[i]);
    }
    result += "}";
    return result;
  }

  void TableStyle::setCellPadding(int16_t cellPadding)
  {
    this->setTopCellPadding(cellPadding);
    this->setRightCellPadding(cellPadding);
    this->setBottomCellPadding(cellPadding);
    this->setLeftCellPadding(cellPadding);
  }
}

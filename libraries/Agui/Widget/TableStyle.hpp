#pragma once
#include "Agui/Style.hpp"
#include "Agui/BorderImageSet.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include <vector>
#include <memory>

namespace agui
{
  class Table;

  class TableStyle : public Style
  {
    using super = Style;
  public:
    explicit TableStyle(const TableStyle* parent = nullptr);
    explicit TableStyle(Table* relatedWidget, const TableStyle* parent = nullptr);
    TableStyle(TableStyle&&) noexcept = delete;
    TableStyle& operator=(TableStyle&&) noexcept = delete;
    const TableStyle* getParent() const { return static_cast<const TableStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    /** Wraps the alignment vector so the getColumnAlignment/setColumnAlignment works properly
     * for cases when the vector is not big enough. */
    class AreaAlignments
    {
    public:
      AreaAlignments(const std::vector<AreaAlign>& data) : data(data) {}
      bool operator==(const AreaAlignments& other) const = default;
      AreaAlign getItem(uint32_t columnIndex) const;
      AreaAlignments copy() const;
      AreaAlignments& setItem(uint32_t columnIndex, agui::AreaAlign value);
      AreaAlignments& setItems(uint32_t startColumnIndex, uint32_t endColumnIndex, agui::AreaAlign value);
      std::string str() const;

    private:
      std::vector<AreaAlign> data;
    };

    class Widths
    {
    public:
      struct Item
      {
        bool empty() const { return this->minimalWidth == 0 && this->maximalWidth == 0; }
        std::string str() const;
        bool operator==(const Item& other) const = default;

        int minimalWidth = 0, maximalWidth = 0;
      };

      Widths() = default;
      Widths(const std::vector<Item>& data) : data(data) {}
      bool operator==(const Widths& other) const = default;
      Item getItem(uint32_t columnIndex) const;
      Widths copy() const;
      void setUnifiedColumnWidth(Item item);
      Widths& setItem(uint32_t columnIndex, Item item);
      const std::vector<Item>& getData() const { return this->data; }
      void clear() { this->unifiedWidth.reset(); this->data.clear(); }
      std::string str() const;

    private:
      std::optional<Item> unifiedWidth;
      std::vector<Item> data;
    };

    class Spacings final
    {
    public:
      Spacings() = default;
      Spacings(const Spacings&) = default;
      Spacings(Spacings&&) = default;
      explicit Spacings(int uniformSpacing) : Spacings(std::make_optional(uniformSpacing), std::vector<int>()) {}
      explicit Spacings(std::vector<int> variableSpacings) : Spacings(std::nullopt, std::move(variableSpacings)) {}
      Spacings(std::optional<int> uniformSpacing, std::vector<int> variableSpacings) : uniformSpacing(uniformSpacing), variableSpacings(std::move(variableSpacings)) {}
      Spacings& operator=(const Spacings&) = default;
      Spacings& operator=(Spacings&&) = default;

      bool operator==(const Spacings& other) const = default;

      const std::vector<int>& getSpacings() const { return this->variableSpacings; }
      int getSpacingAfter(uint32_t index) const;
      int sumSpacingsBetween(uint32_t indexFrom, uint32_t indexTo) const;

      void setUniformSpacing(int spacing) { this->uniformSpacing = spacing; this->variableSpacings.clear(); }
      void clear() { this->uniformSpacing.reset(); this->variableSpacings.clear(); }
      std::string str() const;

      std::optional<int> uniformSpacing;
    private:
      std::vector<int> variableSpacings;
    };

    const Spacings& getHorizontalSpacing() const { return *this->getProperty(&TableStyle::horizontalSpacing); }
    void setHorizontalSpacing(const Spacings& horizontalSpacings) { this->setProperty(&TableStyle::horizontalSpacing, horizontalSpacings); }
    void setHorizontalSpacing(int32_t spacing) { this->setProperty(&TableStyle::horizontalSpacing, Spacings(spacing)); }
    const Spacings& getVerticalSpacing() const { return *this->getProperty(&TableStyle::verticalSpacing); }
    void setVerticalSpacing(const Spacings& verticalSpacings) { this->setProperty(&TableStyle::verticalSpacing, verticalSpacings); }
    void setVerticalSpacing(int32_t spacing) { this->setProperty(&TableStyle::verticalSpacing, Spacings(spacing)); }
    int16_t getTopCellPadding() const { return this->getProperty(&TableStyle::topCellPadding); }
    void setTopCellPadding(int16_t topCellPadding) { this->setProperty(&TableStyle::topCellPadding, topCellPadding); }
    int16_t getRightCellPadding() const { return this->getProperty(&TableStyle::rightCellPadding); }
    void setRightCellPadding(int16_t rightCellPadding) { this->setProperty(&TableStyle::rightCellPadding, rightCellPadding); }
    int16_t getBottomCellPadding() const { return this->getProperty(&TableStyle::bottomCellPadding); }
    void setBottomCellPadding(int16_t bottomCellPadding) { this->setProperty(&TableStyle::bottomCellPadding, bottomCellPadding); }
    int16_t getLeftCellPadding() const { return this->getProperty(&TableStyle::leftCellPadding); }
    void setLeftCellPadding(int16_t leftCellPadding) { this->setProperty(&TableStyle::leftCellPadding, leftCellPadding); }
    void setCellPadding(int16_t cellPadding);
    bool getApplyRowGraphicalSetPerColumn() const { return this->getProperty(&TableStyle::applyRowGraphicalSetPerColumn); }
    void setApplyRowGraphicalSetPerColumn(bool value) { this->setProperty(&TableStyle::applyRowGraphicalSetPerColumn, value); }
    bool getWideAsColumnCount() const { return this->getProperty(&TableStyle::wideAsColumnCount); }
    void setWideAsColumnCount(bool value) { this->setProperty(&TableStyle::wideAsColumnCount, value); }
    const ElementImageSet* getColumnGraphicalSet() const { return this->getPropertyOptional(&TableStyle::columnGraphicalSet); }
    void setColumnGraphicalSet(const ElementImageSet& columnGraphicalSet) { this->setProperty(&TableStyle::columnGraphicalSet, columnGraphicalSet); }
    const ElementImageSet* getDefaultRowGraphicalSet() const { return this->getPropertyOptional(&TableStyle::defaultRowGraphicalSet); }
    void setDefaultRowGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::defaultRowGraphicalSet, value); }
    const ElementImageSet* getEvenRowGraphicalSet() const { return this->getPropertyOptional(&TableStyle::evenRowGraphicalSet); }
    void setEvenRowGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::evenRowGraphicalSet, value); }
    const ElementImageSet* getOddRowGraphicalSet() const { return this->getPropertyOptional(&TableStyle::oddRowGraphicalSet); }
    void setOddRowGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::oddRowGraphicalSet, value); }
    const ElementImageSet* getHoveredGraphicalSet() const { return this->getPropertyOptional(&TableStyle::hoveredGraphicalSet); }
    void setHoveredGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::hoveredGraphicalSet, value); }
    const ElementImageSet* getClickedGraphicalSet() const { return this->getPropertyOptional(&TableStyle::clickedGraphicalSet); }
    void setClickedGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::clickedGraphicalSet, value); }
    const ElementImageSet* getSelectedGraphicalSet() const { return this->getPropertyOptional(&TableStyle::selectedGraphicalSet); }
    void setSelectedGraphicalSet(const ElementImageSet& value) { this->setProperty(&TableStyle::selectedGraphicalSet, value); }
    const ElementImageSet* getSelectedHoveredGraphicalSet() const { return this->getPropertyOptional(&TableStyle::selectedHoveredGraphicalSet); }
    void setSelectedHoveredGraphicalSet(const ElementImageSet& selectedHoveredGraphicalSet) { this->selectedHoveredGraphicalSet.reset(new ElementImageSet(selectedHoveredGraphicalSet)); }
    const ElementImageSet* getSelectedClickedGraphicalSet() const { return this->getPropertyOptional(&TableStyle::selectedClickedGraphicalSet); }
    void setSelectedClickedGraphicalSet(const ElementImageSet& selectedClickedGraphicalSet) { this->selectedClickedGraphicalSet.reset(new ElementImageSet(selectedClickedGraphicalSet)); }
    const ElementImageSet* getRules() const { return this->getPropertyOptional(&TableStyle::rules); }
    void setRules(const ElementImageSet& rules) { this->setProperty(&TableStyle::rules, rules); }
    const ElementImageSet* getBackgroundGraphicalSet() const { return this->getPropertyOptional(&TableStyle::backgroundGraphicalSet); }
    void setBackgroundGraphicalSet(const ElementImageSet& backgroundGraphicalSet) { this->setProperty(&TableStyle::backgroundGraphicalSet, backgroundGraphicalSet); }

    const AreaAlignments* getColumnAlignments() const { return this->getProperty(&TableStyle::columnAlignments); }
    void setColumnAlignments(const AreaAlignments& columnAlignments) { this->setProperty(&TableStyle::columnAlignments, columnAlignments); }
    const Widths* getColumnWidths() const { return this->getProperty(&TableStyle::columnWidths); }
    void setColumnWidths(const Widths& columnWidths) { this->setProperty(&TableStyle::columnWidths, columnWidths); }
    const Color& getHoveredRowColor() const { return *this->getProperty(&TableStyle::hoveredRowColor); }
    void setHoveredRowColor(Color hoveredRowColor) { this->hoveredRowColor.reset(new Color(hoveredRowColor)); }
    const Color& getSelectedRowColor() const { return *this->getProperty(&TableStyle::selectedRowColor); }
    void setSelectedRowColor(Color selectedRowColor) { this->selectedRowColor.reset(new Color(selectedRowColor));}
    const Color& getVerticalLineColor() const { return *this->getProperty(&TableStyle::verticalLineColor); }
    void setVerticalLineColor(Color color) { this->verticalLineColor.reset(new Color(color)); }
    const Color& getHorizontalLineColor() const { return *this->getProperty(&TableStyle::horizontalLineColor); }
    void setHorizontalLineColor(Color color) { this->horizontalLineColor.reset(new Color(color)); }
    const ButtonStyle* getColumnOrderingAscendingButtonStyle() const { return this->getProperty(&TableStyle::columnOrderingAscendingButtonStyle); }
    ButtonStyle* initColumnOrderingAscendingButtonStyle() { return this->initProperty(&TableStyle::columnOrderingAscendingButtonStyle); }
    const ButtonStyle* getColumnOrderingDescendingButtonStyle() const { return this->getProperty(&TableStyle::columnOrderingDescendingButtonStyle); }
    ButtonStyle* initColumnOrderingDescendingButtonStyle() { return this->initProperty(&TableStyle::columnOrderingDescendingButtonStyle); }
    const ButtonStyle* getInactiveColumnOrderingAscendingButtonStyle() const { return this->getProperty(&TableStyle::inactiveColumnOrderingAscendingButtonStyle); }
    ButtonStyle* initInactiveColumnOrderingAscendingButtonStyle() { return this->initProperty(&TableStyle::inactiveColumnOrderingAscendingButtonStyle); }
    const ButtonStyle* getInactiveColumnOrderingDescendingButtonStyle() const { return this->getProperty(&TableStyle::inactiveColumnOrderingDescendingButtonStyle); }
    ButtonStyle* initInactiveColumnOrderingDescendingButtonStyle() { return this->initProperty(&TableStyle::inactiveColumnOrderingDescendingButtonStyle); }
    const BorderImageSet* getBorder() const { return this->getProperty(&TableStyle::borderImageSet); }
    void setBorder(const BorderImageSet& borderImageSet) { this->borderImageSet.reset(new BorderImageSet(borderImageSet)); }
    bool usesBorder() const { return this->getBorder()->isSet; }

  private:
    std::unique_ptr<Spacings> horizontalSpacing;
    std::unique_ptr<Spacings> verticalSpacing;
    std::optional<int16_t> topCellPadding;
    std::optional<int16_t> rightCellPadding;
    std::optional<int16_t> bottomCellPadding;
    std::optional<int16_t> leftCellPadding;
    std::optional<bool> applyRowGraphicalSetPerColumn;
    std::optional<bool> wideAsColumnCount;
    std::unique_ptr<ElementImageSet> columnGraphicalSet; // we either have column graphical sets or row sets
    std::unique_ptr<ElementImageSet> defaultRowGraphicalSet;
    std::unique_ptr<ElementImageSet> evenRowGraphicalSet;
    std::unique_ptr<ElementImageSet> oddRowGraphicalSet;
    std::unique_ptr<ElementImageSet> hoveredGraphicalSet;
    std::unique_ptr<ElementImageSet> clickedGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedHoveredGraphicalSet;
    std::unique_ptr<ElementImageSet> selectedClickedGraphicalSet;
    std::unique_ptr<ElementImageSet> rules;
    std::unique_ptr<ElementImageSet> backgroundGraphicalSet;
    std::unique_ptr<AreaAlignments> columnAlignments;
    std::unique_ptr<Widths> columnWidths;
    std::unique_ptr<Color> hoveredRowColor;
    std::unique_ptr<Color> selectedRowColor;
    std::unique_ptr<Color> verticalLineColor;
    std::unique_ptr<Color> horizontalLineColor;
    std::unique_ptr<ButtonStyle> columnOrderingAscendingButtonStyle;
    std::unique_ptr<ButtonStyle> columnOrderingDescendingButtonStyle;
    std::unique_ptr<ButtonStyle> inactiveColumnOrderingAscendingButtonStyle;
    std::unique_ptr<ButtonStyle> inactiveColumnOrderingDescendingButtonStyle;
    std::unique_ptr<BorderImageSet> borderImageSet;
  };
}

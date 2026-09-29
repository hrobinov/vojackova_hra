#pragma once
#include <vector>
#include <Agui/GuiDirection.hpp>

namespace agui
{
  class Widget;

  // Tool to distribute squashing between different items with different minimal sizes.
  class SquashCalculator
  {
  public:
    struct Item
    {
      Item(GuiDirection direction, Widget& widget);
      void merge(const Item& other);
      void ensureMinimum(int minimum);

      Widget& widget;
      int minimumSize, minimumWithMarginSize;
      int sizeBeforeStretching, sizeBeforeStretchingWithMargin;
      int resultSize, resultWithMarginSize;
    };

    SquashCalculator(GuiDirection direction) : direction(direction) {}
    void add(Widget& widget);
    void merge(Widget& widget);
    void merge(uint32_t index, Widget& widget);
    void calculate(int neededSizeSum);

    std::vector<Item> items;
    GuiDirection direction;
  };
}

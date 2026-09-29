#pragma once
#include <Agui/ValueComparePointer.hpp>

namespace agui { class Dimension; }
namespace agui { class Image; }
namespace agui { class PaintEvent; }
namespace agui { class Point; }
namespace agui { class Rectangle; }

namespace agui
{
  /** The image set is not the owner of the pictures, it just handles copies of the pointers */
  class BorderImageSet
  {
  public:
    // which line endings to use
    enum class Terminal
    {
      None,
      Empty, // keeps the space for the terminal
      TopRightCorner,
      BottomRightCorner,
      BottomLeftCorner,
      TopLeftCorner,
      TopT,
      RightT,
      BottomT,
      LeftT,
      Cross,
      TopEnd,
      RightEnd,
      BottomEnd,
      LeftEnd,
    };

    const Image* getSprite(const Terminal type) const;
    void drawTerminal(const PaintEvent& paintEvent,
                      const Point position,
                      const Terminal terminal) const;

    void drawHorizontalLine(const PaintEvent& paintEvent,
                            const Point start,
                            const int length,
                            const Terminal left = Terminal::LeftEnd,
                            const Terminal right = Terminal::RightEnd) const;
    void drawVerticalLine(const PaintEvent& paintEvent,
                          const Point start,
                          const int length,
                          const Terminal top = Terminal::TopEnd,
                          const Terminal bottom = Terminal::BottomEnd) const;
    void drawBorder(const PaintEvent& paintEvent, const Point start, const Dimension size) const; // rectangular border
    bool operator==(const BorderImageSet& other) const = default;

    bool isSet = false; // whether border should be used with the item
    int lineWidth = 0;

    ValueComparePointer<Image> verticalLine, horizontalLine;
    ValueComparePointer<Image> topRightCorner, bottomRightCorner, bottomLeftCorner, topLeftCorner;
    ValueComparePointer<Image> topT, rightT, bottomT, leftT;
    ValueComparePointer<Image> cross;
    ValueComparePointer<Image> topEnd, rightEnd, bottomEnd, leftEnd;
  };
}

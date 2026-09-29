#pragma once
#include <Agui/Color.hpp>
#include <Agui/ValueComparePointer.hpp>
#include <cstdint>
#include <vector>

namespace agui { class Dimension; }
namespace agui { class Graphics; }
namespace agui { class Image; }
namespace agui { class PaintEvent; }
namespace agui { class Point; }
namespace agui { class Rectangle; }

namespace agui
{
  /** The image set is not the owner of the pictures, it just handles copies of the pointers */
  class ElementImageSet
  {
  public:
    enum class LayerType { Shadow, Base, Glow };
    class Layer
    {
    public:
      enum class Type : uint8_t
      {
        None = 0, // Don't draw any background
        Composition = 1 // Composite background by using corners and stretching sides + center
      };

      enum class DrawType : uint8_t
      {
        Inner = 1, // Border is drawn on the inner part of the element
        Outer = 0 // Border is drawn on the outer of the element
      };
      void draw(const PaintEvent& paintEvent,
                const Rectangle& rectangle,
                const agui::Point& absolutePosition,
                float dynamicOpacity = 1.0f,
                Color tint = Color(1, 1, 1, 1)) const;
      bool operator==(const Layer& other) const = default;
    private:
      bool drawInner(const PaintEvent& paintEvent, const Rectangle& rectangle, const agui::Point& absolutePosition, const Color& tint, float dynamicOpacity) const;
      void drawOuter(const PaintEvent& paintEvent, const Rectangle& rectangle, const agui::Point& absolutePosition, const Color& tint, float dynamicOpacity) const;
    public:

      int getTopBorder() const;
      int getRightBorder() const;
      int getBottomBorder() const;
      int getLeftBorder() const;
      int getHorizontalBorder() const;
      int getVerticalBorder() const;
      int getMinimalHeight() const;
      int getMaxBorder() const;

    private:
      static const uint8_t TILE_HORIZONTALLY = 1 << 0;
      static const uint8_t TILE_VERTICALLY = 1 << 1;
      void drawTiled(Graphics* graphics,
                     const Image* image,
                     const Point& where,
                     const Dimension& totalSize,
                     const Color& tint,
                     float opacity,
                     uint8_t tilingFlags) const;

    public:
      static const uint8_t LEFT_TILING = 1 << 0;
      static const uint8_t RIGHT_TILING = 1 << 1;
      static const uint8_t TOP_TILING = 1 << 2;
      static const uint8_t BOTTOM_TILING = 1 << 3;
      static const uint8_t CENTER_TILING_VERTICAL = 1 << 4;
      static const uint8_t CENTER_TILING_HORIZONTAL = 1 << 5;

      Type type = Type::None;
      DrawType drawType = DrawType::Inner;
      bool stretchMonolithImageToSize = true;
      uint8_t tilingFlags = 0;
      uint16_t overallTilingHorizontalSize = 0;
      uint16_t overallTilingHorizontalSpacing = 0;
      uint16_t overallTilingHorizontalPadding = 0;
      uint16_t overallTilingVerticalSize = 0;
      uint16_t overallTilingVerticalSpacing = 0;
      uint16_t overallTilingVerticalPadding = 0;
      std::vector<uint32_t> customHorizontalSizes;
      std::vector<int32_t> customHorizontalSpacings;
      std::vector<int32_t> customVerticalSpacings;
      int topMonolithBorder = 0;
      int rightMonolithBorder = 0;
      int bottomMonolithBorder = 0;
      int leftMonolithBorder = 0;
      int topOuterBorderShift = 0;
      int bottomOuterBorderShift = 0;
      int leftOuterBorderShift = 0;
      int rightOuterBorderShift = 0;
      ValueComparePointer<Image> leftTop , rightTop, leftBottom, rightBottom;
      ValueComparePointer<Image> top, right, bottom, left;
      ValueComparePointer<Image> center;
      float opacity = 0;
      float backgroundBlurSigma = 0;
    };

    bool operator==(const ElementImageSet& other) const = default;
    int getTopBorder() const { return this->base.getTopBorder(); }
    int getRightBorder() const { return this->base.getRightBorder(); }
    int getBottomBorder() const { return this->base.getBottomBorder(); }
    int getLeftBorder() const { return this->base.getLeftBorder(); }
    int getHorizontalBorder() const { return this->base.getHorizontalBorder(); }
    int getVerticalBorder() const { return this->base.getVerticalBorder(); }
    int getMinimalHeight() const { return this->base.getMinimalHeight(); }
    const Layer& getLayer(LayerType layerType) const;

    Layer shadow, base, glow;
  };
}

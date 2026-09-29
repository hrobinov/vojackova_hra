#include <Agui/ElementImageSet.hpp>
#include <Agui/Gui.hpp>
#include <Agui/Image.hpp>
#include <Agui/PaintEvent.hpp>
#include <Agui/Graphics.hpp>
#include <algorithm>
#include <stdexcept>

namespace agui
{

  void ElementImageSet::Layer::draw(const PaintEvent& paintEvent,
                                    const Rectangle& rectangle,
                                    const agui::Point& absolutePosition,
                                    float opacity,
                                    Color tint) const
  {
    const auto hSpacingAfter = [this](int col)
    {
      return this->overallTilingHorizontalSpacing ?
             this->overallTilingHorizontalSpacing :
             (col < int(this->customHorizontalSpacings.size())) ?
             this->customHorizontalSpacings[col] :
             0;
    };
    const auto vSpacingAfter = [this](int row)
    {
      return this->overallTilingVerticalSpacing ?
             this->overallTilingVerticalSpacing :
             (row < int(this->customVerticalSpacings.size())) ?
             this->customVerticalSpacings[row] :
             0;
    };

    if (this->drawType == DrawType::Inner)
      if (this->overallTilingHorizontalSize == 0 && this->customHorizontalSizes.empty())
        if (this->overallTilingVerticalSize == 0)
          this->drawInner(paintEvent, rectangle, absolutePosition, tint, opacity);
        else
          for (int y = 0, yPos = this->overallTilingVerticalPadding; yPos < rectangle.getHeight(); yPos += (this->overallTilingVerticalSize + vSpacingAfter(y)), y++)
            this->drawInner(paintEvent, Rectangle(rectangle.x + this->overallTilingHorizontalPadding,
                                                  rectangle.y + yPos,
                                                  rectangle.width - this->overallTilingHorizontalPadding * 2,
                                                  this->overallTilingVerticalSize), absolutePosition, tint, opacity);
      else if (this->overallTilingVerticalSize == 0)
        for (int x = 0, xPos = this->overallTilingHorizontalPadding; xPos < rectangle.getWidth(); xPos += (this->overallTilingHorizontalSize + hSpacingAfter(x)), x++)
          this->drawInner(paintEvent, Rectangle(rectangle.x + xPos,
                                                rectangle.y + this->overallTilingVerticalPadding,
                                                this->overallTilingHorizontalSize,
                                                rectangle.height), absolutePosition, tint, opacity);
      else if (!this->customHorizontalSizes.empty())
        for (int x = 0, xPos = this->overallTilingHorizontalPadding; x < int(this->customHorizontalSizes.size()); xPos += (this->customHorizontalSizes[x] + hSpacingAfter(x)), x++)
          for (int y = 0, yPos = this->overallTilingVerticalPadding; yPos < rectangle.getHeight(); yPos += (this->overallTilingVerticalSize + vSpacingAfter(y)), y++)
            this->drawInner(paintEvent, Rectangle(rectangle.x + xPos,
                                                  rectangle.y + yPos,
                                                  this->customHorizontalSizes[x],
                                                  this->overallTilingVerticalSize), absolutePosition, tint, opacity);
      else
      {
        bool firstYSet = false;
        int firstValidY = 0;
        int firstValidYPos = this->overallTilingVerticalPadding;
        for (int x = 0, xPos = this->overallTilingHorizontalPadding; xPos < rectangle.getWidth(); xPos += (this->overallTilingHorizontalSize + hSpacingAfter(x)), x++)
        {
          bool drewInY = false;
          for (int y = firstValidY, yPos = firstValidYPos; yPos < rectangle.getHeight(); yPos += (this->overallTilingVerticalSize + vSpacingAfter(y)), y++)
          {
            Rectangle areaToDraw = Rectangle(rectangle.x + xPos,
                                             rectangle.y + yPos,
                                             this->overallTilingHorizontalSize,
                                             this->overallTilingVerticalSize);
            if (this->drawInner(paintEvent, areaToDraw, absolutePosition, tint, opacity))
            {
              if (!firstYSet)
              {
                firstYSet = true;
                firstValidY = y;
                firstValidYPos = yPos;
              }
              drewInY = true;
            }
            else if (drewInY)
              break;
          }
        }
      }
    else
      this->drawOuter(paintEvent, rectangle, absolutePosition, tint, opacity);
  }

  bool ElementImageSet::Layer::drawInner(const PaintEvent& paintEvent, const Rectangle& rectangle, const agui::Point& absolutePosition, const Color& tint, float dynamicOpacity) const
  {
    if (!paintEvent.graphics()->isClippingRectEmpty())
      if (!paintEvent.graphics()->getClippingRectangle().collide(rectangle + absolutePosition))
        return false;

    if (this->backgroundBlurSigma > 0)
    {
      if (!paintEvent.graphics()->isClippingRectEmpty())
      {
        const Rectangle clipRectangle = paintEvent.graphics()->getClippingRectangle() - absolutePosition;
        int x1 = std::max(clipRectangle.getLeft(), rectangle.getLeft());
        int x2 = std::min(clipRectangle.getRight(), rectangle.getRight());
        int y1 = std::max(clipRectangle.getTop(), rectangle.getTop());
        int y2 = std::min(clipRectangle.getBottom(), rectangle.getBottom());
        if (x2 >= x1 && y2 >= y1)
        {
          const Rectangle toBlur = Rectangle(Point(x1, y1), Point(x2, y2));
          paintEvent.graphics()->blurRectangle(toBlur, this->backgroundBlurSigma);
        }
      }
      else
        paintEvent.graphics()->blurRectangle(rectangle, this->backgroundBlurSigma);
    }

    switch (this->type)
    {
      case Type::Composition:
      if (this->left || this->right || this->top || this->bottom)
      {
        int leftBorder = (this->leftTop ? this->leftTop->getWidth() : this->left ? this->left->getWidth() : 0) * Gui::scale;
        int topBorder = (this->leftTop ? this->leftTop->getHeight() : this->rightTop ? this->rightTop->getHeight() : this->top ? this->top->getHeight() : 0) * Gui::scale;
        int bottomBorder = (this->leftBottom ? this->leftBottom->getHeight() : this->rightBottom ? this->rightBottom->getHeight() : this->bottom ? this->bottom->getHeight() : 0) * Gui::scale;
        int rightBorder = (this->rightTop ? this->rightTop->getWidth() : this->right ? this->right->getWidth() : 0) * Gui::scale;
        int centerWidth = rectangle.getRight() - rectangle.getLeft() - leftBorder - rightBorder;
        int centerHeight = rectangle.getBottom() - rectangle.getTop() - topBorder - bottomBorder;
        Point previousOffset = paintEvent.graphics()->getOffset();
        Graphics* graphics = paintEvent.graphics();
        graphics->setOffset(Point(previousOffset.x + rectangle.getLeft(), previousOffset.y + rectangle.getTop()));

        if (this->left)
          if (this->leftTop && this->leftBottom)
          {
            paintEvent.graphics()->drawScaledTintedImage(this->leftTop,
                                                         Point(0, 0),
                                                         Dimension(leftBorder, topBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            this->drawTiled(graphics,
                            this->left,
                            Point(0, topBorder),
                            Dimension(leftBorder, centerHeight),
                            tint,
                            this->opacity * dynamicOpacity,
                            (this->tilingFlags & LEFT_TILING) ? TILE_VERTICALLY : 0);
            paintEvent.graphics()->drawScaledTintedImage(this->leftBottom,
                                                         Point(0, topBorder + centerHeight),
                                                         Dimension(leftBorder, bottomBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          }
          else if (this->leftTop)
          {
            paintEvent.graphics()->drawScaledTintedImage(this->leftTop,
                                                         Point(0, 0),
                                                         Dimension(leftBorder, topBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            this->drawTiled(graphics,
                            this->left,
                            Point(0, topBorder),
                            Dimension(leftBorder, centerHeight),
                            tint,
                            this->opacity * dynamicOpacity,
                            (this->tilingFlags & LEFT_TILING) ? TILE_VERTICALLY : 0);
          }
          else if (this->leftBottom)
          {
            this->drawTiled(graphics,
                            this->left,
                            Point(0, topBorder),
                            Dimension(leftBorder, centerHeight),
                            tint,
                            this->opacity * dynamicOpacity,
                            (this->tilingFlags & LEFT_TILING) ? TILE_VERTICALLY : 0);
            paintEvent.graphics()->drawScaledTintedImage(this->leftBottom,
                                                         Point(0, topBorder + centerHeight),
                                                         Dimension(leftBorder, bottomBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          }
          else
            this->drawTiled(graphics,
                            this->left,
                            Point(0, 0),
                            Dimension(leftBorder, rectangle.getHeight()),
                            tint,
                            this->opacity * dynamicOpacity,
                            (this->tilingFlags & LEFT_TILING) != 0 ? TILE_VERTICALLY : 0);

        this->drawTiled(graphics,
                        this->center,
                        Point(leftBorder, this->top ? topBorder : 0),
                        Dimension(centerWidth,
                                  centerHeight + (this->top ? 0 : topBorder) + (this->bottom ? 0 : bottomBorder)),
                        tint,
                        this->opacity * dynamicOpacity,
                        ((this->tilingFlags & CENTER_TILING_HORIZONTAL) ? TILE_HORIZONTALLY : 0) |
                        ((this->tilingFlags & CENTER_TILING_VERTICAL) ? TILE_VERTICALLY : 0));

        if (this->right)
          if (this->rightTop && this->rightBottom)
          {
            paintEvent.graphics()->drawScaledTintedImage(this->rightTop,
                                                         Point(rectangle.getWidth() - rightBorder, 0),
                                                         Dimension(rightBorder, topBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->right,
                                                         Point(rectangle.getWidth() - rightBorder, topBorder),
                                                         Dimension(rightBorder, centerHeight),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->rightBottom,
                                                         Point(rectangle.getWidth() - rightBorder, topBorder + centerHeight),
                                                         Dimension(rightBorder, bottomBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          }
          else if (this->rightTop)
          {
            paintEvent.graphics()->drawScaledTintedImage(this->rightTop,
                                                         Point(rectangle.getWidth() - rightBorder, 0),
                                                         Dimension(rightBorder, topBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->right,
                                                         Point(rectangle.getWidth() - rightBorder, topBorder),
                                                         Dimension(rightBorder, centerHeight),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          }
          else if (this->rightBottom)
          {
            paintEvent.graphics()->drawScaledTintedImage(this->right,
                                                         Point(rectangle.getWidth() - rightBorder, topBorder),
                                                         Dimension(rightBorder, centerHeight),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->rightBottom,
                                                         Point(rectangle.getWidth() - rightBorder, topBorder + centerHeight),
                                                         Dimension(rightBorder, bottomBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          }
          else
            paintEvent.graphics()->drawScaledTintedImage(this->right,
                                                         Point(rectangle.getWidth() - rightBorder, 0),
                                                         Dimension(rightBorder, rectangle.getHeight()),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
        if (this->top)
          this->drawTiled(graphics,
                          this->top,
                          Point(leftBorder, 0),
                          Dimension(centerWidth, topBorder),
                          tint,
                          this->opacity * dynamicOpacity,
                          (this->tilingFlags & TOP_TILING) ? TILE_HORIZONTALLY : 0);
        if (this->bottom)
          this->drawTiled(graphics,
                          this->bottom,
                          Point(leftBorder, topBorder + centerHeight),
                          Dimension(centerWidth, bottomBorder),
                          tint,
                          this->opacity * dynamicOpacity,
                          (this->tilingFlags & BOTTOM_TILING) ? TILE_HORIZONTALLY : 0);

        paintEvent.graphics()->setOffset(previousOffset);
      }
      else
      {
        if (this->stretchMonolithImageToSize)
          paintEvent.graphics()->drawScaledTintedImage(this->center, rectangle.getLeftTop(), rectangle.getSize(), tint, this->opacity * dynamicOpacity);
        else
        {
          int targetWidth = this->center->getWidth();
          int targetHeight = this->center->getHeight();

          if (rectangle.width < targetWidth)
          {
            targetHeight = targetHeight * float(rectangle.width) / float(targetWidth);
            targetWidth = rectangle.width;
          }

          if (rectangle.height < targetHeight)
          {
            targetWidth = targetWidth * float(rectangle.height) / float(targetHeight);
            targetHeight = rectangle.height;
          }

          paintEvent.graphics()->drawScaledTintedImage(this->center,
                                                       Point((rectangle.width - targetWidth) / 2,
                                                             (rectangle.height - targetHeight) / 2),
                                                       Dimension(targetWidth, targetHeight),
                                                       tint);
        }
      }
      break;
      case Type::None:
        break;
    }

    return true;
  }

  void ElementImageSet::Layer::drawOuter(const PaintEvent& paintEvent, const Rectangle& rectangle, const agui::Point& absolutePosition, const Color& tint, float dynamicOpacity) const
  {
    if (!paintEvent.graphics()->isClippingRectEmpty())
      if (!paintEvent.graphics()->getClippingRectangle().collide(rectangle + absolutePosition))
        return;

    switch (this->type)
    {
      case Type::Composition:
        if (this->left || this->right || this->top || this->bottom)
        {
          int leftBorder = (this->leftTop ? this->leftTop->getWidth() : this->left ? this->left->getWidth() : 0) * Gui::scale;
          int topBorder = (this->leftTop ? this->leftTop->getHeight() : this->rightTop ? this->rightTop->getHeight() : this->top ? this->top->getHeight() : 0) * Gui::scale;
          int bottomBorder = (this->leftBottom ? this->leftBottom->getHeight() : this->rightBottom ? this->rightBottom->getHeight() : this->bottom ? this->bottom->getHeight() : 0) * Gui::scale;
          int rightBorder = (this->rightTop ? this->rightTop->getWidth() : this->right ? this->right->getWidth() : 0) * Gui::scale;
          int centerWidth = rectangle.getRight() - rectangle.getLeft() + (this->rightOuterBorderShift - this->leftOuterBorderShift) * Gui::scale;
          int verticalCorrection = (this->bottomOuterBorderShift - this->topOuterBorderShift ) * Gui::scale;
          int centerHeight = rectangle.getBottom() - rectangle.getTop() + verticalCorrection;
          Point previousOffset = paintEvent.graphics()->getOffset();
          paintEvent.graphics()->setOffset(Point(previousOffset.x + rectangle.getLeft() - leftBorder + leftOuterBorderShift * Gui::scale,
                                                 previousOffset.y + rectangle.getTop() - topBorder + topOuterBorderShift * Gui::scale));

          if (this->left)
          {
            if (this->leftTop)
              paintEvent.graphics()->drawScaledTintedImage(this->leftTop,
                                                           Point(0, 0),
                                                           Dimension(this->leftTop->getWidth() * Gui::scale, this->leftTop->getHeight() * Gui::scale),
                                                           tint,
                                                           this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->left,
                                                         Point(0, leftTop ? topBorder : 0),
                                                         Dimension(this->left->getWidth() * Gui::scale, rectangle.getHeight() + (leftTop ? 0 : topBorder) + (leftBottom ? 0 : bottomBorder) + verticalCorrection),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            if (this->leftBottom)
              paintEvent.graphics()->drawScaledTintedImage(this->leftBottom,
                                                           Point(0, topBorder + centerHeight),
                                                           Dimension(leftBorder, bottomBorder),
                                                           tint,
                                                           this->opacity * dynamicOpacity);
          }

          if (this->right)
          {
            if (this->rightTop)
              paintEvent.graphics()->drawScaledTintedImage(this->rightTop,
                                                           Point(leftBorder + centerWidth, 0),
                                                           Dimension(this->rightTop->getWidth() * Gui::scale, this->rightTop->getHeight() * Gui::scale),
                                                           tint,
                                                           this->opacity * dynamicOpacity);
            paintEvent.graphics()->drawScaledTintedImage(this->right,
                                                         Point(leftBorder + centerWidth + rightBorder - this->right->getWidth() * Gui::scale, this->rightTop ? topBorder : 0),
                                                         Dimension(this->right->getWidth() * Gui::scale, rectangle.getHeight() + (rightTop ? 0 : topBorder) + (rightBottom ? 0 : bottomBorder) + verticalCorrection),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
            if (this->rightBottom)
              paintEvent.graphics()->drawScaledTintedImage(this->rightBottom,
                                                           Point(leftBorder + centerWidth, topBorder + centerHeight),
                                                           Dimension(rightBorder, bottomBorder),
                                                           tint,
                                                           this->opacity * dynamicOpacity);
          }

          if (this->top)
            paintEvent.graphics()->drawScaledTintedImage(this->top,
                                                         Point(leftBorder, 0),
                                                         Dimension(centerWidth, this->top->getHeight() * Gui::scale),
                                                         tint,
                                                         this->opacity * dynamicOpacity);
          if (this->bottom)
            paintEvent.graphics()->drawScaledTintedImage(this->bottom,
                                                         Point(leftBorder, topBorder + centerHeight),
                                                         Dimension(centerWidth, bottomBorder),
                                                         tint,
                                                         this->opacity * dynamicOpacity);

          paintEvent.graphics()->drawScaledTintedImage(this->center,
                                                       Point(leftBorder, this->top ? topBorder : 0),
                                                       Dimension(centerWidth,
                                                                 centerHeight + (this->top ? 0 : topBorder) + (this->bottom ? 0 : bottomBorder)),
                                                       tint,
                                                       this->opacity * dynamicOpacity);

          paintEvent.graphics()->setOffset(previousOffset);
        }
        else // only center is drawn
        {
          if (this->stretchMonolithImageToSize)
          {
            Point previousOffset = paintEvent.graphics()->getOffset();
            paintEvent.graphics()->setOffset(Point(previousOffset.x + rectangle.getLeft() + this->leftOuterBorderShift * Gui::scale,
                                                   previousOffset.y + rectangle.getTop() + this->topOuterBorderShift * Gui::scale));
            paintEvent.graphics()->drawScaledTintedImage(this->center,
                                                         Point(0, 0),
                                                         Dimension(rectangle.getWidth() + (this->rightOuterBorderShift - this->leftOuterBorderShift) * Gui::scale,
                                                                   rectangle.getHeight() + (this->bottomOuterBorderShift - this->topOuterBorderShift) * Gui::scale),
                                                         tint);
            paintEvent.graphics()->setOffset(previousOffset);
          }
          else
          {
            int targetWidth = this->center->getWidth() * Gui::instance->scale;
            int targetHeight = this->center->getHeight() * Gui::instance->scale;

            paintEvent.graphics()->drawScaledTintedImage(this->center,
                                                         Point(rectangle.getLeft() + (rectangle.width - targetWidth) / 2,
                                                               rectangle.getTop() + (rectangle.height - targetHeight) / 2),
                                                         Dimension(targetWidth, targetHeight),
                                                         tint);
          }
        }
        break;
      case Type::None:
        break;
    }
  }

  int ElementImageSet::Layer::getTopBorder() const
  {
    switch (this->type)
    {
      case Type::Composition: return this->topMonolithBorder * Gui::scale;
      case Type::None: return 0;
      default: return 0;
    }
  }

  int ElementImageSet::Layer::getRightBorder() const
  {
    switch (this->type)
    {
      case Type::Composition: return this->rightMonolithBorder * Gui::scale;
      case Type::None: return 0;
      default: return 0;
    }
  }

  int ElementImageSet::Layer::getBottomBorder() const
  {
    switch (this->type)
    {
      case Type::Composition: return this->bottomMonolithBorder * Gui::scale;
      case Type::None: return 0;
      default: return 0;
    }
  }

  int ElementImageSet::Layer::getLeftBorder() const
  {
    switch (this->type)
    {
      case Type::Composition: return this->leftMonolithBorder * Gui::scale;
      case Type::None: return 0;
      default: return 0;
    }
  }

  int ElementImageSet::Layer::getHorizontalBorder() const
  {
    return this->getLeftBorder() + this->getRightBorder();
  }

  int ElementImageSet::Layer::getVerticalBorder() const
  {
    return this->getTopBorder() + this->getBottomBorder();
  }

  int ElementImageSet::Layer::getMinimalHeight() const
  {
    if (this->type != Type::Composition)
      return 0;
    if (this->getTopBorder() > 0 && this->getBottomBorder() > 0)
      return this->getTopBorder() + this->getBottomBorder();
    if (this->left)
      return this->left->getHeight() * Gui::scale;
    return 0;
  }

  int ElementImageSet::Layer::getMaxBorder() const
  {
    return std::max({this->getTopBorder(), this->getRightBorder(), this->getBottomBorder(), this->getLeftBorder(),
                     this->getHorizontalBorder(), this->getVerticalBorder()});
  }

  void ElementImageSet::Layer::drawTiled(Graphics* graphics,
                                         const Image* image,
                                         const Point& where,
                                         const Dimension& totalSize,
                                         const Color& tint,
                                         float opacity,
                                         uint8_t tilingFlags) const
  {
    if (image->getHeight() == 0 || image->getWidth() == 0)
      return;

    int rightEdge = where.x + totalSize.width - image->getWidth() * Gui::instance->scale;
    int bottomEdge = where.y + totalSize.height - image->getHeight() * Gui::instance->scale;

    if (tilingFlags & TILE_HORIZONTALLY)
      if (tilingFlags & TILE_VERTICALLY)
        for (int y = where.y; y <= bottomEdge; y += image->getHeight() * Gui::instance->scale)
          for (int x = where.x; x <= rightEdge; x += image->getWidth() * Gui::instance->scale)
            graphics->drawTintedImage(image, Point(x, y), Point(0, 0), Dimension(image->getWidth() * Gui::instance->scale, image->getHeight() * Gui::instance->scale), tint, opacity);
      else
        for (int x = where.x; x <= rightEdge; x += image->getWidth() * Gui::instance->scale)
          graphics->drawScaledTintedImage(image, Point(x, where.y), Dimension(image->getWidth() * Gui::instance->scale, totalSize.height), tint, opacity);
    else
      if (tilingFlags & TILE_VERTICALLY)
        for (int y = where.y; y <= bottomEdge; y += image->getHeight() * Gui::instance->scale)
          graphics->drawScaledTintedImage(image, Point(where.x, y), Dimension(totalSize.width, image->getHeight() * Gui::instance->scale), tint, opacity);
      else
        graphics->drawScaledTintedImage(image, Point(where.x, where.y), totalSize, tint, opacity);
  }

  const ElementImageSet::Layer& ElementImageSet::getLayer(LayerType layerType) const
  {
    switch (layerType)
    {
      case LayerType::Shadow: return this->shadow;
      case LayerType::Base: return this->base;
      case LayerType::Glow: return this->glow;
    }
    throw std::runtime_error("Invalid layer type");
  }
}

#pragma once
#include "Agui/Rectangle.hpp"
#include "Agui/AlignmentEnum.hpp"
#include "Agui/TextEnums.hpp"
#include "Agui/Font.hpp"
#include "Agui/Effect.hpp"
#include <vector>
namespace agui
{
  using InvertColors = NamedBool<class InvertColorsTag>;

  class Color;
  class Font;
  class Image;
  class ResizableText;
  class Widget;
}

namespace agui
{
  /**
   * Abstract class for Graphics and drawing methods.
   *
   * Must implement:
   *
   * _beginPaint
   * _endPaint
   * setClippingRectangle
   * getDisplaySize
   * getClippingRectangle
   * drawImage
   * drawScaledImage
   * drawText
   * drawRectangle
   * drawFilledRectangle
   * drawImage
   * drawCircle
   * drawFilledCircle
   * drawPixel
   * drawLine
   * setTargetImage
   * resetTargetImage */
  class Graphics
  {
    void setClippedRectangleToLastApplied();
    void setForcedClippedRectangleToLastForced();
  public:
    Rectangle getForcedRectangle() const;
  protected:
    virtual void setClippingRectangle(const Rectangle& rect, bool force) = 0;

  public:
    /** Called before a widget is painted. */
    virtual void _beginPaint() = 0;
    /** Called after a widget is painted. */
    virtual void _endPaint() = 0;
    /** @return The offset used internally to simulate relative painting.
     * All drawing coordinates must be added to this value. */
    const Point& getOffset() const;
    /** Sets the offset used internally to simulate relative painting.
     * All drawing coordinates must be added to this value. */
    void setOffset(const Point& offset);
    Graphics() {}
    virtual ~Graphics() {}
    /** @return The size of the native display / window. */
    virtual Dimension getDisplaySize() const = 0;
    virtual Rectangle getClippingRectangle() = 0;
    virtual bool isClippingRectEmpty();
    /** Pushes the parameter rectangle onto the clipping stack.
     * The resulting clipping rectangle will be an intersection of all the clipping rectangles.
     * Returns true if this intersection is non-degenerate */
    bool pushClippingRect(const agui::Widget* widget, Rectangle rect, bool force = false, const agui::Point* customOffset = nullptr);
    /** Pushes full screen rect on the stack and enables the full screen for drawing.
     * This overwrites the drawing restrictions on the stack.*/
    void pushFullScreenClippingRect();
    void popClippingRect();
    void clearClippingStack();
    virtual void drawImage(const Image* bmp,
                           const Point& position,
                           const Point& regionStart,
                           const Dimension& regionSize,
                           const float& opacity = 1.0f) = 0;
    virtual void drawTintedImage(const Image* bmp,
                                 const Point& position,
                                 const Point& regionStart,
                                 const Dimension& regionSize,
                                 const Color& tint,
                                 const float& opacity = 1.0f) = 0;
    virtual void drawImage(const Image* bmp,
                           const Point& position,
                           const float& opacity = 1.0f,
                           InvertColors invertColors = InvertColors::False) = 0;
    virtual void drawScaledImage(const Image* bmp,
                                 const Point& position,
                                 const Dimension& scale,
                                 const float& opacity = 1.0f,
                                 InvertColors invertColors = InvertColors::False) = 0;
    virtual void drawScaledImageRegion(const Image* bmp,
                                       const Point& position,
                                       const Dimension& scale,
                                       const agui::Rectangle& imageRegion,
                                       const float& opacity = 1.0f,
                                       InvertColors invertColors = InvertColors::False) = 0;
    virtual void drawScaledTintedImage(const Image* bmp,
                                       const Point& position,
                                       const Dimension& scale,
                                       const Color& tint,
                                       const float& opacity = 1.0f,
                                       InvertColors invertColors = InvertColors::False) = 0;
    virtual void drawScaledRotatedTintedImage(const Image* bmp,
                                              double centerX, double centerY,
                                              const float orientation,
                                              const Point& regionStart,
                                              const Dimension& regionScale,
                                              const Dimension& scale,
                                              const Color& tint,
                                              const float& opacity = 1.0f) = 0;

    /* Draws a UTF8 encoded string.
     * @param position Where to text's top left pixel will be.
     * It may also be the center or end of the text depending on the alignment. */
    virtual void drawText(const Point& position,
                          const std::string& text,
                          const Color& color,
                          const Font* font,
                          agui::RichTextSetting richTextSetting,
                          HorizontalAlign align = HorizontalAlign::Left,
                          const TextHighlightErrorColors& highlightColors = TextHighlightErrorColors::defaults(),
                          HighlightedText highlighted = HighlightedText::False,
                          float scale = 1.0f,
                          VerticalAlign verticalAlign = VerticalAlign::Top,
                          float orientation = 0.0f) = 0;

    virtual void drawTextLines(const Point& position,
                               std::string_view text,
                               const Color& color,
                               const Font* font,
                               agui::RichTextSetting richTextSetting = agui::RichTextSetting::Enabled,
                               HorizontalAlign align = HorizontalAlign::Left) = 0;
    virtual void drawTextLines(const ResizableText& text,
                               std::vector<std::pair<size_t, agui::Point>>& linePositions,
                               const Font* font,
                               const Color& color,
                               const agui::TextHighlightErrorColors& highlightColors = agui::TextHighlightErrorColors::defaults());
    virtual void drawRectangle(const Rectangle& rect, const Color& color, int width = 1) = 0; // Draws the outline of a rectangle
    virtual void drawFilledRectangle(const Rectangle& rect, const Color& color) = 0;
    virtual void drawFilledTriangle(const Point& a, const Point& b, const Point& c, const Color& color) = 0;
    virtual void drawCircle(const Point& center, float radius, const Color& color, double width = 1) = 0;
    virtual void drawFilledCircle(const Point& center, float radius, const Color& color) = 0;
    virtual void drawFilledPieSlice(const Point& center, float radius, float startTheta, float deltaTheta, const Color& color) = 0;
    virtual void drawPixel(const Point& point, const Color& color) = 0;
    virtual void drawLine(const Point& start, const Point& end, const Color& color, float thickness = 1.0) = 0;
    virtual void drawPolyline(const std::vector<Point>& points, const Color& color, float width = 0) = 0;
    virtual void blurRectangle(const Rectangle& rect, float intensity) = 0;
    virtual void drawGradientBorderOverlay(const Rectangle& rect) = 0;

    virtual void beginEffect() {}
    virtual void endEffect(const agui::Effect* effect, float opacity) { (void)effect; (void)opacity; }

    virtual Rectangle rectangleForCentering(const Widget&) { return Rectangle(Point(0, 0), this->getDisplaySize()); }
    virtual void setTint(const agui::Color& color) = 0;
    virtual agui::Color getTint() = 0;
    void setClippingRectangleToTheLastForced();

    static const Rectangle FULL_SCREEN_RECTANGLE; // magic value signaling that there is no clip
  private:
    struct ClipData
    {
      Rectangle rectangle;
      bool applied;
      bool force;
    };
    std::vector<ClipData> clipStack;
    Rectangle clipRectangle;
    Point offset;
  public:
    bool debugView = false;
    bool shadowView = true;
    bool glowView = true;

    std::vector<std::pair<size_t, Point>> scratchLinePositions;
  };
}

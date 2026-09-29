#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/ImageStyle.hpp"
#include <memory>
namespace agui { class Image; }

namespace agui
{
  /** Image as widget. */
  class ImageWidget : public Widget
  {
    using super = Widget;
  public:
    ImageWidget(std::unique_ptr<Image> image, bool scale, const ImageStyle* parentStyle = &ImageWidget::defaultStyle);
    ImageWidget(std::unique_ptr<Image> image, const ImageStyle* parentStyle = &ImageWidget::defaultStyle);
    ImageWidget(const ImageStyle* parentStyle = &ImageWidget::defaultStyle);
    virtual ~ImageWidget();
    virtual void logic(double timeElapsed) override;
    virtual Style* getStyle() override { return &this->style; }
    void load(std::unique_ptr<Image> image);
    bool isLoaded() const;
    virtual void clear() override;
    Image* getImage();
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo) override;
    void setImage(std::unique_ptr<Image> image);
  protected:
    void paintImage(const PaintEvent&, agui::Image* image);
    virtual void paintComponent(const PaintEvent& paintEvent, const Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition) override;
    virtual void resizeToContents() override;
  public:

    static ImageStyle defaultStyle;
  private:
    std::unique_ptr<Image> image;
  public:
    ImageStyle style;
    bool scale = false;
    bool scaleWithGuiScale = false;
    bool cropToKeepTheRatio = false;
    bool scaleToKeepTheRatio = false;
    double fixedRatio = 0;
    double opacity = 1;
  };
}

#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/Widget/ImageWidget.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/PaintEvent.hpp"
#include <math.h>

#include <cmath>

namespace agui
{
  ImageStyle ImageWidget::defaultStyle;

  ImageWidget::~ImageWidget() = default;

  void ImageWidget::logic(double timeElapsed)
  {
    if (this->image)
      this->image->logic(timeElapsed);
  }

  ImageWidget::ImageWidget(std::unique_ptr<Image> image, bool scale, const ImageStyle* parentStyle)
    : image(std::move(image))
    , style(this, parentStyle)
    , scale(scale)
  {}

  ImageWidget::ImageWidget(std::unique_ptr<Image> image, const ImageStyle* parentStyle)
    : image(std::move(image))
    , style(this, parentStyle)
  {}

  ImageWidget::ImageWidget(const ImageStyle* parentStyle)
    : style(this, parentStyle)
  {}

  void ImageWidget::load(std::unique_ptr<Image> image)
  {
    const bool shouldResize = bool(this->image) != bool(image) ||
                              (this->image && image &&
                               (this->image->getWidth() != image->getWidth() || this->image->getHeight() != image->getHeight()));
    this->image = std::move(image);
    if (shouldResize)
      this->triggerResize();
  }

  bool ImageWidget::isLoaded() const
  {
    return this->image != nullptr;
  }

  void ImageWidget::paintComponent(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->paintImage(paintEvent, this->image.get());
    if (paintEvent.graphics()->shadowView)
      this->style.getGraphicalSet()->shadow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void ImageWidget::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->style.getGraphicalSet()->base.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void ImageWidget::resizeToContents()
  {
    if (this->image && !this->scale)
      this->setContentSize(this->isHorizontallySquashable() ? 0 : this->image->getWidth(),
                           this->isVerticallyStretchable() ? 0 : this->image->getHeight());
    else
      super::resizeToContents();
  }

  void ImageWidget::paintImage(const PaintEvent& paintEvent, agui::Image* image)
  {
    Dimension contentSize = this->getContentSize();
    InvertColors invertColors;
    if (this->isParentHovered() && this->style.getInvertColorsOfPictureWhenHoveredOrToggled())
      invertColors = InvertColors::True;
    if (contentSize.width == 0 || contentSize.height == 0)
      return;
    if (image != nullptr)
    {
      if (this->scaleToKeepTheRatio)
      {
        double contentSizeRatio = double(contentSize.width) / contentSize.height;
        double imageRatio = double(image->getWidth()) / image->getHeight();
        if (imageRatio < contentSizeRatio) // image is not wide enough
          paintEvent.graphics()->drawScaledImage(image,
                                                 Point(0.5f * (contentSize.width - (contentSize.height * imageRatio)), 0),
                                                 Dimension(contentSize.height * imageRatio, contentSize.height),
                                                 this->opacity,
                                                 invertColors);
        else // image is too wide
          paintEvent.graphics()->drawScaledImage(image,
                                                 Point(0, 0.5f * (contentSize.height - (contentSize.width * (1.0f / imageRatio)))),
                                                 Dimension(contentSize.width, contentSize.width * (1.0f / imageRatio)),
                                                 this->opacity,
                                                 invertColors);
      }
      else if (this->scale || this->style.getStretchImageToWidgetSize() || this->scaleWithGuiScale)
      {
        if (this->cropToKeepTheRatio)
        {
          double contentSizeRatio = double(contentSize.width) / contentSize.height;
          double imageRatio = double(image->getWidth()) / image->getHeight();
          if (fabs(contentSizeRatio - imageRatio) > 0.01)
          {
            if (imageRatio < contentSizeRatio) // image is not wide enough
            {
              double newImageHeight = image->getWidth() / contentSizeRatio;
              paintEvent.graphics()->drawScaledImageRegion(image,
                                                           Point(0, 0),
                                                           contentSize,
                                                           agui::Rectangle(0, (image->getHeight() - newImageHeight) / 2,
                                                                           image->getWidth(), newImageHeight),
                                                           this->opacity,
                                                           invertColors);
            }
            else // image is too wide
            {
              double newImageWidth = image->getHeight() * contentSizeRatio;
              paintEvent.graphics()->drawScaledImageRegion(image,
                                                           Point(0, 0),
                                                           contentSize,
                                                           agui::Rectangle((image->getWidth() - newImageWidth) / 2, 0,
                                                                           newImageWidth, image->getHeight()),
                                                           this->opacity,
                                                           invertColors);
            }
          }
          else
            paintEvent.graphics()->drawScaledImage(image,
                                                   Point(0, 0),
                                                   contentSize,
                                                   this->opacity,
                                                   invertColors);
        }
        else
          paintEvent.graphics()->drawScaledImage(image,
                                                 Point(0, 0),
                                                 contentSize,
                                                 this->opacity,
                                                 invertColors);
      }
      else
        paintEvent.graphics()->drawImage(image, Point(0, 0), this->opacity, invertColors);
    }
  }

  void ImageWidget::clear()
  {
    this->image.reset();
    this->triggerResize();
  }

  Image* ImageWidget::getImage()
  {
    return this->image.get();
  }

  void ImageWidget::setSize(int width, int height, SetSizeInfo setSizeInfo)
  {
    if (this->image)
    {
      if (this->scaleWithGuiScale && this->getGui())
        if (Gui* gui = this->getGui())
        {
          width = int(width * gui->scale);
          height = int(height * gui->scale);
        }

      double ratio = this->fixedRatio ? this->fixedRatio : (double(this->image->getWidth()) / double(this->image->getHeight()));

      // making sure that the aspect ratio is kept the same when it is being squashed
      if (setSizeInfo.vertical == Change::Squashing)
        width = height * ratio;
      if (setSizeInfo.horizontal == Change::Squashing)
      {
        int newHeight = width / ratio;
        if (newHeight < height)
          height = newHeight;
        else
          width = height * ratio;
      }
      if (setSizeInfo.horizontal == Change::Stretching)
        height = width / ratio;
    }
    super::setSize(width, height, setSizeInfo);
  }

  void ImageWidget::setImage(std::unique_ptr<Image> image)
  {
    this->image = std::move(image);
  }
}

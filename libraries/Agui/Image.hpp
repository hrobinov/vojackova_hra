#pragma once
#include "Agui/Dimension.hpp"

namespace agui
{
  class Image
  {
  public:
    Image() = default;
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    virtual ~Image() = default;

    virtual void logic(double timeElapsed) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    /** Determines if the Image will destroy the back end specific image when
     * the image is changed or deleted. */
    virtual bool isAutoFreeing() const = 0;
    virtual void free() = 0;

    Dimension getDimension() const { return Dimension(this->getWidth(), this->getHeight()); }

    virtual bool operator==(const Image& other) const = 0;

    Image& operator=(const Image&) = delete;
    Image& operator=(Image&&) = delete;
  };
}

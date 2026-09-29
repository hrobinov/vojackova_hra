#pragma once
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include <memory>

namespace agui
{
  class ImageWidget;

  class ImageStyle : public Style
  {
    using super = Style;
  public:
    explicit ImageStyle(const ImageStyle* parent = nullptr);
    explicit ImageStyle(ImageWidget* relatedWidget, const ImageStyle* parent = nullptr);
    const ImageStyle* getParent() const { return static_cast<const ImageStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const ElementImageSet* getGraphicalSet() const { return this->getProperty(&ImageStyle::graphicalSet); }
    void setGraphicalSet(ElementImageSet* graphicalSet) { this->graphicalSet.reset(new ElementImageSet(*graphicalSet)); }
    bool getStretchImageToWidgetSize() const { return this->getProperty(&ImageStyle::stretchImageToWidgetSize); }
    void setStretchImageToWidgetSize(const bool& stretchImageToWidgetSize) { this->stretchImageToWidgetSize = stretchImageToWidgetSize; }
    bool getInvertColorsOfPictureWhenHoveredOrToggled() const { return this->getProperty(&ImageStyle::invertColorsOfPictureWhenHoveredOrToggled); }
    void setInvertColorsOfPictureWhenHoveredOrToggled(bool invertColorsOfPictureWhenHoveredOrToggled) { this->invertColorsOfPictureWhenHoveredOrToggled = invertColorsOfPictureWhenHoveredOrToggled; }

  private:
    std::unique_ptr<ElementImageSet> graphicalSet;
    std::optional<bool> stretchImageToWidgetSize;
    std::optional<bool> invertColorsOfPictureWhenHoveredOrToggled;
  };
}

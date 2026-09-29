#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/BorderImageSet.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Widget/EmptyWidgetStyle.hpp"
#include "Agui/Widget/FlowStyle.hpp"
#include "Agui/Widget/HorizontalFlowStyle.hpp"
#include "Agui/Widget/LabelStyle.hpp"
#include "Agui/Widget/VerticalFlowStyle.hpp"
#include <memory>

namespace agui
{
  class Font;
  class Frame;

  class FrameStyle : public Style
  {
    using super = Style;
  public:
    explicit FrameStyle(const FrameStyle* parent = nullptr);
    explicit FrameStyle(Frame* relatedWidget, const FrameStyle* parent = nullptr);
    const FrameStyle* getParent() const { return static_cast<const FrameStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const ElementImageSet* getGraphicalSet() const { return this->getProperty(&FrameStyle::graphicalSet); }
    void setGraphicalSet(const ElementImageSet* graphicalSet) { this->graphicalSet.reset(new ElementImageSet(*graphicalSet)); }

    bool getUseHeaderFiller() const { return this->getProperty(&FrameStyle::useHeaderFiller); }
    void setUseHeaderFiller(bool useHeaderFiller) { this->useHeaderFiller = useHeaderFiller; }
    const ElementImageSet* getHeaderBackground() const { return this->getPropertyOptional(&FrameStyle::headerBackground); }
    void setHeaderBackground(const ElementImageSet* headerBackground) { this->headerBackground.reset(new ElementImageSet(*headerBackground)); }
    const ElementImageSet* getBackgroundGraphicalSet() const { return this->getPropertyOptional(&FrameStyle::backgroundGraphicalSet); }
    void setBackgroundGraphicalSet(const ElementImageSet* backgroundGraphicalSet) { this->backgroundGraphicalSet.reset(new ElementImageSet(*backgroundGraphicalSet)); }

    const VerticalFlowStyle* getVerticalFlowStyle() const { return this->getProperty(&FrameStyle::verticalFlowStyle); }
    VerticalFlowStyle* initVerticalFlowStyle() { return this->initProperty(&FrameStyle::verticalFlowStyle); }
    const HorizontalFlowStyle* getHorizontalFlowStyle() const { return this->getProperty(&FrameStyle::horizontalFlowStyle); }
    HorizontalFlowStyle* initHorizontalFlowStyle() { return this->initProperty(&FrameStyle::horizontalFlowStyle); }
    const HorizontalFlowStyle* getHeaderFlowStyle() const { return this->getProperty(&FrameStyle::headerFlowStyle); }
    HorizontalFlowStyle* initHeaderFlowStyle() { return this->initProperty(&FrameStyle::headerFlowStyle); }
    const EmptyWidgetStyle* getHeaderFillerStyle() const { return this->getProperty(&FrameStyle::headerFillerStyle); }
    EmptyWidgetStyle* initHeaderFillerStyle() { return this->initProperty(&FrameStyle::headerFillerStyle); }
    const LabelStyle* getTitleStyle() const { return this->getProperty(&FrameStyle::titleStyle); }
    LabelStyle* initTitleStyle() { return this->initProperty(&FrameStyle::titleStyle); }
    bool usesBorder() const { return this->getBorder()->isSet; }
    const BorderImageSet* getBorder() const { return this->getProperty(&FrameStyle::borderImageSet); }
    void setBorder(const BorderImageSet& borderImageSet) { this->borderImageSet.reset(new BorderImageSet(borderImageSet)); }
    bool getDragByTitle() const { return this->getProperty(&FrameStyle::dragByTitle); }
    void setDragByTitle(bool value) { this->dragByTitle = value; }

  private:
    std::unique_ptr<ElementImageSet> graphicalSet;
    std::optional<bool> useHeaderFiller;
    std::optional<bool> dragByTitle;
    std::unique_ptr<ElementImageSet> headerBackground;
    std::unique_ptr<ElementImageSet> backgroundGraphicalSet;

    std::unique_ptr<VerticalFlowStyle> verticalFlowStyle;
    std::unique_ptr<HorizontalFlowStyle> horizontalFlowStyle;
    std::unique_ptr<HorizontalFlowStyle> headerFlowStyle;
    std::unique_ptr<EmptyWidgetStyle> headerFillerStyle;
    std::unique_ptr<LabelStyle> titleStyle;
    std::unique_ptr<BorderImageSet> borderImageSet;
  };
}

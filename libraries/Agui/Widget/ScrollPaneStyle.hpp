#pragma once
#include "Agui/Style.hpp"
#include "Agui/Widget/HorizontalFlowStyle.hpp"
#include "Agui/Widget/VerticalFlowStyle.hpp"
#include <Agui/Widget/ScrollBarStyle.hpp>

namespace agui
{
  class ScrollPane;

  class ScrollPaneStyle : public Style
  {
    using super = Style;
  public:
    explicit ScrollPaneStyle(const ScrollPaneStyle* parent = nullptr);
    explicit ScrollPaneStyle(ScrollPane* relatedWidget, const ScrollPaneStyle* parent = nullptr);
    const ScrollPaneStyle* getParent() const { return static_cast<const ScrollPaneStyle*>(this->parent); }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const override;
    virtual void clear() override;

    const VerticalFlowStyle* getVerticalFlowStyle() const { return this->getProperty(&ScrollPaneStyle::verticalFlowStyle); }
    VerticalFlowStyle* initVerticalFlowStyle() { return this->initProperty(&ScrollPaneStyle::verticalFlowStyle); }

    const ScrollBarStyle* getHorizontalScrollBarStyle() const { return this->getProperty(&ScrollPaneStyle::horizontalScrollBarStyle); }
    ScrollBarStyle* initHorizontalScrollBarStyle() { return this->initProperty(&ScrollPaneStyle::horizontalScrollBarStyle); }

    const ScrollBarStyle* getVerticalScrollBarStyle() const { return this->getProperty(&ScrollPaneStyle::verticalScrollBarStyle); }
    ScrollBarStyle* initVerticalScrollBarStyle() { return this->initProperty(&ScrollPaneStyle::verticalScrollBarStyle); }

    const ElementImageSet* getGraphicalSet() const { return this->getProperty(&ScrollPaneStyle::graphicalSet); }
    void initGraphicalSet(const ElementImageSet* graphicalSet) { this->graphicalSet.reset(new ElementImageSet(*graphicalSet)); }

    const ElementImageSet* getBackgroundGraphcialSet() const { return this->getProperty(&ScrollPaneStyle::backgroundGraphicalSet); }
    void initBackgroundGraphicalSet(const ElementImageSet* backgroundGraphicalSet) { this->backgroundGraphicalSet.reset(new ElementImageSet(*backgroundGraphicalSet)); }

    int getExtraTopPaddingWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraTopPaddingWhenActivated); }
    void setExtraTopPaddingWhenActivated(int value) { this->extraTopPaddingWhenActivated = value; }

    int getExtraBottomPaddingWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraBottomPaddingWhenActivated); }
    void setExtraBottomPaddingWhenActivated(int value) { this->extraBottomPaddingWhenActivated = value; }

    int getExtraLeftPaddingWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraLeftPaddingWhenActivated); }
    void setExtraLeftPaddingWhenActivated(int value) { this->extraLeftPaddingWhenActivated = value; }

    int getExtraRightPaddingWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraRightPaddingWhenActivated); }
    void setExtraRightPaddingWhenActivated(int value) { this->extraRightPaddingWhenActivated = value; }

    void setExtraPaddingWhenActivated(int value);

    int getExtraTopMarginWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraTopMarginWhenActivated); }
    void setExtraTopMarginWhenActivated(int value) { this->extraTopMarginWhenActivated = value; }

    int getExtraBottomMarginWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraBottomMarginWhenActivated); }
    void setExtraBottomMarginWhenActivated(int value) { this->extraBottomMarginWhenActivated = value; }

    int getExtraLeftMarginWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraLeftMarginWhenActivated); }
    void setExtraLeftMarginWhenActivated(int value) { this->extraLeftMarginWhenActivated = value; }

    int getExtraRightMarginWhenActivated() const { return this->getProperty(&ScrollPaneStyle::extraRightMarginWhenActivated); }
    void setExtraRightMarginWhenActivated(int value) { this->extraRightMarginWhenActivated = value; }

    bool shouldNotForceClippingRectForContents() const { return this->getProperty(&ScrollPaneStyle::dontForceClippingRectForContents); }
    //this allows the contentFlow to draw outside of our ScrollPane, when the ScrollPane is NOT active. Used in the Character Gui.
    void setDontForceClippingRectForContents(bool dontForceClippingRectForContents) { this->dontForceClippingRectForContents = dontForceClippingRectForContents; }

    bool getAlwaysDrawBorders() const { return this->getProperty(&ScrollPaneStyle::alwaysDrawBorders); }
    void setAlwaysDrawBorders(bool alwaysDrawBorders) { this->alwaysDrawBorders = alwaysDrawBorders; }

    bool getScrollbarsGoOutside() const { return this->getProperty(&ScrollPaneStyle::scrollbarsGoOutside); }
    void setScrollbarsGoOutside(bool scrollbarsGoOutside) { this->scrollbarsGoOutside = scrollbarsGoOutside; }

  private:
    std::unique_ptr<VerticalFlowStyle> verticalFlowStyle;
    std::unique_ptr<ScrollBarStyle> horizontalScrollBarStyle;
    std::unique_ptr<ScrollBarStyle> verticalScrollBarStyle;
    std::unique_ptr<ElementImageSet> graphicalSet; // only used when active
    std::unique_ptr<ElementImageSet> backgroundGraphicalSet;

    std::optional<int> extraTopPaddingWhenActivated;
    std::optional<int> extraBottomPaddingWhenActivated;
    std::optional<int> extraLeftPaddingWhenActivated;
    std::optional<int> extraRightPaddingWhenActivated;

    std::optional<int> extraTopMarginWhenActivated;
    std::optional<int> extraBottomMarginWhenActivated;
    std::optional<int> extraLeftMarginWhenActivated;
    std::optional<int> extraRightMarginWhenActivated;
    std::optional<bool> dontForceClippingRectForContents;
    std::optional<bool> alwaysDrawBorders;
    std::optional<bool> scrollbarsGoOutside;
  };
}

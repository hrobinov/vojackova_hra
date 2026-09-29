#include "Agui/Widget/ScrollPaneStyle.hpp"
#include "Agui/Widget/ScrollPane.hpp"

namespace agui
{

  ScrollPaneStyle::ScrollPaneStyle(const ScrollPaneStyle* parent)
    : ScrollPaneStyle(nullptr, parent)
  {}

  ScrollPaneStyle::ScrollPaneStyle(ScrollPane* relatedWidget, const ScrollPaneStyle* parent)
    : Style(relatedWidget, parent, false)
  {
    if (this->relatedWidget)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void ScrollPaneStyle::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    super::addChangedValues(result, comparedWith);
    if (this->extraTopPaddingWhenActivated && (this->parent != 0 || this->extraTopPaddingWhenActivated != 0))
      this->addStyleComment(result, "extra_top_padding_when_activated", std::to_string(*this->extraTopPaddingWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraTopPaddingWhenActivated));
    if (this->extraBottomPaddingWhenActivated && (this->parent != 0 || this->extraBottomPaddingWhenActivated != 0))
      this->addStyleComment(result, "extra_bottom_padding_when_activated", std::to_string(*this->extraBottomPaddingWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraBottomPaddingWhenActivated));
    if (this->extraLeftPaddingWhenActivated && (this->parent != 0 || this->extraLeftPaddingWhenActivated != 0))
      this->addStyleComment(result, "extra_left_padding_when_activated", std::to_string(*this->extraLeftPaddingWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraLeftPaddingWhenActivated));
    if (this->extraRightPaddingWhenActivated && (this->parent != 0 || this->extraRightPaddingWhenActivated != 0))
      this->addStyleComment(result, "extra_right_padding_when_activated", std::to_string(*this->extraRightPaddingWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraRightPaddingWhenActivated));
    if (this->extraTopMarginWhenActivated && (this->parent != 0 || this->extraTopMarginWhenActivated != 0))
      this->addStyleComment(result, "extra_top_margin_when_activated", std::to_string(*this->extraTopMarginWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraTopMarginWhenActivated));
    if (this->extraBottomMarginWhenActivated && (this->parent != 0 || this->extraBottomMarginWhenActivated != 0))
      this->addStyleComment(result, "extra_bottom_margin_when_activated", std::to_string(*this->extraBottomMarginWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraBottomMarginWhenActivated));
    if (this->extraLeftMarginWhenActivated && (this->parent != 0 || this->extraLeftMarginWhenActivated != 0))
      this->addStyleComment(result, "extra_left_margin_when_activated", std::to_string(*this->extraLeftMarginWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraLeftMarginWhenActivated));
    if (this->extraRightMarginWhenActivated && (this->parent != 0 || this->extraRightMarginWhenActivated != 0))
      this->addStyleComment(result, "extra_right_margin_when_activated", std::to_string(*this->extraRightMarginWhenActivated), this->propertyStatus(comparedWith, &ScrollPaneStyle::extraRightMarginWhenActivated));
    if (this->dontForceClippingRectForContents.has_value() && (this->parent != 0 || this->dontForceClippingRectForContents != false))
      this->addStyleComment(result, "dont_force_clipping_rect_for_contents", std::to_string(*this->dontForceClippingRectForContents), this->propertyStatus(comparedWith, &ScrollPaneStyle::dontForceClippingRectForContents));
    if (this->alwaysDrawBorders.has_value() && (this->parent != 0 || this->alwaysDrawBorders != false))
      this->addStyleComment(result, "always_draw_borders", std::to_string(*this->alwaysDrawBorders), this->propertyStatus(comparedWith, &ScrollPaneStyle::alwaysDrawBorders));
    if (this->scrollbarsGoOutside.has_value() && (this->parent != 0 || this->scrollbarsGoOutside != false))
      this->addStyleComment(result, "scrollbars_go_outside", std::to_string(*this->scrollbarsGoOutside), this->propertyStatus(comparedWith, &ScrollPaneStyle::scrollbarsGoOutside));

    if (this->graphicalSet && this->parent)
      this->addStyleComment(result, "graphical_set", "redefined", this->propertyStatus(comparedWith, &ScrollPaneStyle::graphicalSet));
    if (this->backgroundGraphicalSet && this->parent)
      this->addStyleComment(result, "background_graphical_set", "redefined", this->propertyStatus(comparedWith, &ScrollPaneStyle::backgroundGraphicalSet));
  }

  void ScrollPaneStyle::clear()
  {
    super::clear();
    this->verticalFlowStyle.reset();
    this->horizontalScrollBarStyle.reset();
    this->verticalScrollBarStyle.reset();
    this->graphicalSet.reset();
    this->backgroundGraphicalSet.reset();

    this->extraTopPaddingWhenActivated.reset();
    this->extraBottomPaddingWhenActivated.reset();
    this->extraLeftPaddingWhenActivated.reset();
    this->extraRightPaddingWhenActivated.reset();

    this->extraTopMarginWhenActivated.reset();
    this->extraBottomMarginWhenActivated.reset();
    this->extraLeftMarginWhenActivated.reset();
    this->extraRightMarginWhenActivated.reset();
    this->dontForceClippingRectForContents.reset();
    this->alwaysDrawBorders.reset();
    this->scrollbarsGoOutside.reset();
  }

  void ScrollPaneStyle::setExtraPaddingWhenActivated(int value)
  {
    this->extraTopPaddingWhenActivated = value;
    this->extraRightPaddingWhenActivated = value;
    this->extraBottomPaddingWhenActivated = value;
    this->extraLeftPaddingWhenActivated = value;
  }
}

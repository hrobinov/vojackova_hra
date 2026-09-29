#include "Agui/Gui.hpp"
#include "Agui/Style.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include "Agui/AreaAlignmentEnum.hpp"
#include <Agui/Util.hpp>
#include <Agui/StringUtil.hpp>
#include <stdexcept>
#include <Agui/Sound.hpp>

namespace agui
{
  bool StylePropertyComparators::equal(const Sound* a, const Sound* b)
  {
    return a == b || a->getName() == b->getName();
  }

  Style::Style(const Style* parent)
    : parent(parent)
  {}

  Style::Style(Widget* relatedWidget, const Style* parent, bool isLeaf)
    : parent(parent)
    , relatedWidget(relatedWidget)
  {
    if (this->relatedWidget && isLeaf)
      this->relatedWidget->applySizeRestrictionsInternal(*this);
  }

  void Style::setParent(const Style* parent)
  {
    if (this->parent == parent)
      return;
    this->parent = parent;
    if (this->relatedWidget)
    {
      this->relatedWidget->reapplySubStyles();
      this->relatedWidget->applySizeRestrictions(*this);
    }
  }

  void Style::clear()
  {
    this->definedFlags = 0;
    this->valueFlags = 0;
    this->horizontalAlign = HorizontalAlign::Left;
    this->verticalAlign = VerticalAlign::Top;
    this->minimalWidth = 0;
    this->minimalHeight = 0;
    this->naturalWidth = 0;
    this->naturalHeight = 0;
    this->maximalWidth = 0;
    this->maximalHeight = 0;
    this->topPadding = 0;
    this->rightPadding = 0;
    this->bottomPadding = 0;
    this->leftPadding = 0;
    this->topMargin = 0;
    this->rightMargin = 0;
    this->bottomMargin = 0;
    this->leftMargin = 0;
    this->effect = nullptr;
    this->effectOpacity = 1;
  }

  void Style::setParentSilent(const Style* parent)
  {
    this->parent = parent;
  }

  void Style::setSize(int width, int height)
  {
    this->setMinimalWidth(width);
    this->setMaximalWidth(width);
    this->setMinimalHeight(height);
    this->setMaximalHeight(height);
  }

  void Style::setNaturalSize(int width, int height)
  {
    if (width != 0)
      this->setNaturalWidth(width);
    if (height != 0)
      this->setNaturalHeight(height);
  }

  bool Style::isIgnoredBySearch() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & IGNORED_BY_SEARCH_DEFINED) == 0)
      style = style->parent;
    return style ? (style->valueFlags & IGNORED_BY_SEARCH_VALUE) != 0 : false;
  }

  void Style::setIgnoredBySearch(bool ignoredBySearch)
  {
    this->definedFlags |= IGNORED_BY_SEARCH_DEFINED;
    if (ignoredBySearch)
      this->valueFlags |= IGNORED_BY_SEARCH_VALUE;
    else
      this->valueFlags &= ~IGNORED_BY_SEARCH_VALUE;
    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  bool Style::canBeHiddenBySearch() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & NEVER_HIDE_BY_SEARCH_DEFINED) == 0)
      style = style->parent;
    return style ? (style->valueFlags & NEVER_HIDE_BY_SEARCH_VALUE) == 0 : true;
  }

  void Style::setNeverHiddenBySearch(bool neverHideBySearch)
  {
    this->definedFlags |= NEVER_HIDE_BY_SEARCH_DEFINED;
    if (neverHideBySearch)
      this->valueFlags |= NEVER_HIDE_BY_SEARCH_VALUE;
    else
      this->valueFlags &= ~NEVER_HIDE_BY_SEARCH_VALUE;
    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  const int32_t* Style::getMinimalWidthOptional() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & MINIMAL_WIDTH_DEFINED) == 0)
      style = style->parent;
    return style ? &style->minimalWidth : nullptr;
  }

  const int32_t* Style::getMaximalHeightOptional() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & MAXIMAL_HEIGHT_DEFINED) == 0)
      style = style->parent;
    return style ? &style->maximalHeight : nullptr;
  }

  const int32_t* Style::getMinimalHeightOptional() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & MINIMAL_HEIGHT_DEFINED) == 0)
      style = style->parent;
    return style ? &style->minimalHeight : nullptr;
  }

  PropertyStatus Style::propertyStatus(const Style* comparedWith, uint32_t flag) const
  {
    for (const Style* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
      if ((parent->definedFlags & flag) != 0)
      {
        if (this->flagValueIsTheSame(parent, flag))
          return PropertyStatus::OverwritesTheSame;
        break;
      }

    for (;comparedWith != this; comparedWith = comparedWith->getParent())
      if ((comparedWith->definedFlags & flag) != 0)
        return PropertyStatus::Overwritten;
    return PropertyStatus::Defines;
  }

  bool Style::flagValueIsTheSame(const Style* other, uint32_t flag) const
  {
    switch (flag)
    {
      case HORIZONTAL_ALIGN_DEFINED: return this->horizontalAlign == other->horizontalAlign;
      case VERTICAL_ALIGN_DEFINED: return this->verticalAlign == other->verticalAlign;
      case MINIMAL_WIDTH_DEFINED: return this->minimalWidth == other->minimalWidth;
      case MINIMAL_HEIGHT_DEFINED: return this->minimalHeight == other->minimalHeight;
      case MAXIMAL_WIDTH_DEFINED: return this->maximalWidth == other->maximalWidth ;
      case MAXIMAL_HEIGHT_DEFINED: return this->maximalHeight == other->maximalHeight;
      case TOP_PADDING_DEFINED: return this->topPadding == other->topPadding;
      case RIGHT_PADDING_DEFINED: return this->rightPadding == other->rightPadding;
      case BOTTOM_PADDING_DEFINED: return this->bottomPadding == other->bottomPadding;
      case LEFT_PADDING_DEFINED: return this->leftPadding == other->leftPadding;
      case TOP_MARGIN_DEFINED: return this->topMargin == other->topMargin;
      case RIGHT_MARGIN_DEFINED: return this->rightMargin == other->rightMargin;
      case BOTTOM_MARGIN_DEFINED: return this->bottomMargin == other->bottomMargin;
      case LEFT_MARGIN_DEFINED: return this->leftMargin == other->leftMargin;
      case HORIZONTALLY_STRETCHABLE_DEFINED: return this->isHorizontallyStretchable() == other->isHorizontallyStretchable();
      case VERTICALLY_STRETCHABLE_DEFINED: return this->isVerticallyStretchable() == other->isVerticallyStretchable();
      case HORIZONTALLY_SQUASHABLE_DEFINED: return this->isHorizontallySquashable() == other->isHorizontallySquashable();
      case VERTICALLY_SQUASHABLE_DEFINED: return this->isVerticallySquashable() == other->isVerticallySquashable();
      case NATURAL_WIDTH_DEFINED: return this->naturalWidth == other->naturalWidth;
      case NATURAL_HEIGHT_DEFINED: return this->naturalHeight == other->naturalHeight;
      case IGNORED_BY_SEARCH_DEFINED: return this->isIgnoredBySearch() == other->isIgnoredBySearch();
      case NEVER_HIDE_BY_SEARCH_DEFINED: return this->canBeHiddenBySearch() == other->canBeHiddenBySearch();
    }
    return false;
  }

  void Style::setMinimalWidth(int32_t minimalWidth)
  {
    this->minimalWidth = minimalWidth;
    this->definedFlags |= MINIMAL_WIDTH_DEFINED;

    // increase maximal width if it is lower than minimal
    if (const int32_t* maximalWidth = this->getMaximalWidthOptional())
      if (*maximalWidth > 0 && minimalWidth > *maximalWidth)
        this->setMaximalWidth(minimalWidth);

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::clearMinimalWidth()
  {
    this->minimalWidth = 0;
    this->definedFlags &= ~MINIMAL_WIDTH_DEFINED;

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::setMinimalHeight(int32_t minimalHeight)
  {
    this->minimalHeight = minimalHeight;
    this->definedFlags |= MINIMAL_HEIGHT_DEFINED;

    // increase maximal height if it is lower than minimal
    if (const int32_t* maximalHeight = this->getMaximalHeightOptional())
      if (*maximalHeight > 0 && minimalHeight > *maximalHeight)
        this->setMaximalHeight(minimalHeight);

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::clearMinimalHeight()
  {
    this->minimalHeight = 0;
    this->definedFlags &= ~MINIMAL_HEIGHT_DEFINED;

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  const int32_t* Style::getMaximalWidthOptional() const
  {
    const Style* style = this;
    while (style && (style->definedFlags & MAXIMAL_WIDTH_DEFINED) == 0)
      style = style->parent;
    return style ? &style->maximalWidth : nullptr;
  }

  void Style::setMaximalWidth(int32_t maximalWidth)
  {
    this->maximalWidth = maximalWidth;
    this->definedFlags |= MAXIMAL_WIDTH_DEFINED;

    // decrease minimal width if it is higher than minimal
    if (const int32_t* minimalWidth = this->getMinimalWidthOptional())
      if (maximalWidth > 0 && *minimalWidth > maximalWidth)
        this->setMinimalWidth(maximalWidth);

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::clearMaximalWidth()
  {
    this->maximalWidth = 0;
    this->definedFlags &= ~MAXIMAL_WIDTH_DEFINED;

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::setMaximalHeight(int32_t maximalHeight)
  {
    if (this->maximalHeight == maximalHeight && (this->definedFlags & MAXIMAL_HEIGHT_DEFINED))
      return;

    this->maximalHeight = maximalHeight;
    this->definedFlags |= MAXIMAL_HEIGHT_DEFINED;

    // decrease minimal height if it is higher than minimal
    if (const int32_t* minimalHeight = this->getMinimalHeightOptional())
      if (maximalHeight > 0 && *minimalHeight > maximalHeight)
        this->setMinimalHeight(maximalHeight);

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::clearMaximalHeight()
  {
    this->maximalHeight = 0;
    this->definedFlags &= ~MAXIMAL_HEIGHT_DEFINED;

    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::clearSizes()
  {
    this->clearMinimalWidth();
    this->clearMinimalHeight();
    this->clearMaximalWidth();
    this->clearMaximalHeight();
  }

  void Style::setConstantWidth(int32_t width)
  {
    if (width == this->minimalWidth &&
        width == this->maximalWidth)
      return;
    this->minimalWidth = this->maximalWidth = width;
    this->definedFlags |= MINIMAL_WIDTH_DEFINED | MAXIMAL_WIDTH_DEFINED;
    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::setConstantHeight(int32_t height)
  {
    if (height == this->minimalHeight &&
        height == this->maximalHeight)
      return;
    this->minimalHeight = this->maximalHeight = height;
    this->definedFlags |= MINIMAL_HEIGHT_DEFINED | MAXIMAL_HEIGHT_DEFINED;
    if (this->relatedWidget)
    {
      this->relatedWidget->applySizeRestrictions(*this);
      this->relatedWidget->triggerResize();
    }
  }

  void Style::setPaddings(int16_t topPadding, int16_t rightPadding, int16_t bottomPadding, int16_t leftPadding)
  {
    this->setTopPadding(topPadding);
    this->setRightPadding(rightPadding);
    this->setBottomPadding(bottomPadding);
    this->setLeftPadding(leftPadding);
  }

  void Style::setPadding(int16_t padding)
  {
    this->topPadding = this->rightPadding = this->bottomPadding = this->leftPadding = padding;
    this->definedFlags |= TOP_PADDING_DEFINED | RIGHT_PADDING_DEFINED | BOTTOM_PADDING_DEFINED | LEFT_PADDING_DEFINED;
    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  void Style::setMargin(int16_t margin)
  {
    this->topMargin = this->rightMargin = this->bottomMargin = this->leftMargin = margin;
    this->definedFlags |= TOP_MARGIN_DEFINED | RIGHT_MARGIN_DEFINED | BOTTOM_MARGIN_DEFINED | LEFT_MARGIN_DEFINED;
    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  void Style::addChangedValues(VerticalFlow& result, const Style* comparedWith) const
  {
    if ((this->definedFlags & MINIMAL_WIDTH_DEFINED) != 0 &&
        (this->definedFlags & MAXIMAL_WIDTH_DEFINED) != 0 &&
        this->minimalWidth == this->maximalWidth)
    {
      if (this->getParent() || this->minimalWidth != 0) // don't show the non-affecting sizes in parent
        this->addStyleComment(result,
                              "width",
                              std::to_string(this->minimalWidth),
                              this->propertyStatus(comparedWith, MINIMAL_WIDTH_DEFINED) || this->propertyStatus(comparedWith, MAXIMAL_WIDTH_DEFINED));
    }
    else
    {
      if ((this->definedFlags & MINIMAL_WIDTH_DEFINED) != 0 &&
          (this->getParent() || this->minimalWidth != 0))
        this->addStyleComment(result, "minimal_width", std::to_string(this->minimalWidth), this->propertyStatus(comparedWith, MINIMAL_WIDTH_DEFINED));
      if ((this->definedFlags & MAXIMAL_WIDTH_DEFINED) != 0 &&
          (this->getParent() || this->maximalWidth != 0))
        this->addStyleComment(result, "maximal_width", std::to_string(this->maximalWidth), this->propertyStatus(comparedWith, MAXIMAL_WIDTH_DEFINED));
    }
    if ((this->definedFlags & MINIMAL_HEIGHT_DEFINED) != 0 &&
        (this->definedFlags & MAXIMAL_HEIGHT_DEFINED) != 0 &&
        this->minimalHeight == this->maximalHeight)
    {
      if ((this->getParent() || this->minimalHeight != 0)) // don't show the non-affecting sizes in parent
        this->addStyleComment(result,
                              "height",
                              std::to_string(this->minimalHeight),
                              this->propertyStatus(comparedWith, MINIMAL_HEIGHT_DEFINED) || this->propertyStatus(comparedWith, MAXIMAL_HEIGHT_DEFINED));
    }
    else
    {
      if ((this->definedFlags & MINIMAL_HEIGHT_DEFINED) != 0 &&
          (this->getParent() || this->minimalHeight != 0))
        this->addStyleComment(result, "minimal_height", std::to_string(this->minimalHeight), this->propertyStatus(comparedWith, MINIMAL_HEIGHT_DEFINED));
      if ((this->definedFlags & MAXIMAL_HEIGHT_DEFINED) != 0 &&
          (this->getParent() || this->maximalHeight != 0))
        this->addStyleComment(result, "maximal_height", std::to_string(this->maximalHeight), this->propertyStatus(comparedWith, MAXIMAL_HEIGHT_DEFINED));
    }
    if ((this->definedFlags & NATURAL_WIDTH_DEFINED) != 0 &&
          (this->getParent() || this->naturalWidth != 0))
        this->addStyleComment(result, "natural_width", std::to_string(this->naturalWidth), this->propertyStatus(comparedWith, NATURAL_WIDTH_DEFINED));
    if ((this->definedFlags & NATURAL_HEIGHT_DEFINED) != 0 &&
        (this->getParent() || this->naturalHeight!= 0))
      this->addStyleComment(result, "natural_height", std::to_string(this->naturalHeight), this->propertyStatus(comparedWith, NATURAL_HEIGHT_DEFINED));

    if (this->definedFlags & IGNORED_BY_SEARCH_DEFINED)
      this->addStyleComment(result, "ignored_by_search", StringUtil::boolToString(this->isIgnoredBySearch()), this->propertyStatus(comparedWith, IGNORED_BY_SEARCH_DEFINED));

    if (this->definedFlags & NEVER_HIDE_BY_SEARCH_DEFINED)
      this->addStyleComment(result, "never_hide_by_search", StringUtil::boolToString(!this->canBeHiddenBySearch()), this->propertyStatus(comparedWith, NEVER_HIDE_BY_SEARCH_DEFINED));

    if (this->definedFlags & HORIZONTAL_ALIGN_DEFINED &&
        (this->getParent() || this->horizontalAlign != HorizontalAlign::Left))
      this->addStyleComment(result, "horizontal_align", std::string(horizontalAlignToString(this->horizontalAlign)), this->propertyStatus(comparedWith, HORIZONTAL_ALIGN_DEFINED));

    if (this->definedFlags & VERTICAL_ALIGN_DEFINED &&
        (this->getParent() || this->verticalAlign != VerticalAlign::Top))
      this->addStyleComment(result, "vertical_align", std::string(verticalAlignToString(this->verticalAlign)), this->propertyStatus(comparedWith, VERTICAL_ALIGN_DEFINED));

    if ((this->definedFlags & TOP_PADDING_DEFINED) != 0 &&
        (this->definedFlags & BOTTOM_PADDING_DEFINED) != 0 &&
        (this->definedFlags & LEFT_PADDING_DEFINED) != 0 &&
        (this->definedFlags & RIGHT_PADDING_DEFINED) != 0 &&
        this->topPadding == this->bottomPadding &&
        this->topPadding == this->leftPadding &&
        this->topPadding == this->rightPadding)
    {
      if (this->getParent() || this->topPadding != 0) // don't show the non-affecting sizes in parent
        this->addStyleComment(result, "padding",
                              std::to_string(this->topPadding),
                              this->propertyStatus(comparedWith, TOP_PADDING_DEFINED) ||
                              this->propertyStatus(comparedWith, BOTTOM_PADDING_DEFINED) ||
                              this->propertyStatus(comparedWith, LEFT_PADDING_DEFINED) ||
                              this->propertyStatus(comparedWith, RIGHT_PADDING_DEFINED));
    }
    else
    {
      if ((this->definedFlags & TOP_PADDING_DEFINED) != 0)
        this->addStyleComment(result, "top_padding", std::to_string(this->topPadding), this->propertyStatus(comparedWith, TOP_PADDING_DEFINED));
      if ((this->definedFlags & BOTTOM_PADDING_DEFINED) != 0)
        this->addStyleComment(result, "bottom_padding", std::to_string(this->bottomPadding), this->propertyStatus(comparedWith, BOTTOM_PADDING_DEFINED));
      if ((this->definedFlags & LEFT_PADDING_DEFINED) != 0)
        this->addStyleComment(result, "left_padding", std::to_string(this->leftPadding), this->propertyStatus(comparedWith, LEFT_PADDING_DEFINED));
      if ((this->definedFlags & RIGHT_PADDING_DEFINED) != 0)
        this->addStyleComment(result, "right_padding", std::to_string(this->rightPadding), this->propertyStatus(comparedWith, RIGHT_PADDING_DEFINED));
    }

    if ((this->definedFlags & TOP_MARGIN_DEFINED) != 0 &&
        (this->definedFlags & BOTTOM_MARGIN_DEFINED) != 0 &&
        (this->definedFlags & LEFT_MARGIN_DEFINED) != 0 &&
        (this->definedFlags & RIGHT_MARGIN_DEFINED) != 0 &&
        this->topMargin == this->bottomMargin &&
        this->topMargin == this->leftMargin &&
        this->topMargin == this->rightMargin)
    {
      if (this->getParent() || this->topMargin != 0) // don't show the non-affecting sizes in parent
        this->addStyleComment(result, "margin",
                              std::to_string(this->topMargin),
                              this->propertyStatus(comparedWith, TOP_MARGIN_DEFINED) ||
                              this->propertyStatus(comparedWith, BOTTOM_MARGIN_DEFINED) ||
                              this->propertyStatus(comparedWith, LEFT_MARGIN_DEFINED) ||
                              this->propertyStatus(comparedWith, RIGHT_MARGIN_DEFINED));
    }
    else
    {
      if ((this->definedFlags & TOP_MARGIN_DEFINED) != 0)
        this->addStyleComment(result, "top_margin", std::to_string(this->topMargin), this->propertyStatus(comparedWith, TOP_MARGIN_DEFINED));
      if ((this->definedFlags & BOTTOM_MARGIN_DEFINED) != 0)
        this->addStyleComment(result, "bottom_margin", std::to_string(this->bottomMargin), this->propertyStatus(comparedWith, BOTTOM_MARGIN_DEFINED));
      if ((this->definedFlags & LEFT_MARGIN_DEFINED) != 0)
        this->addStyleComment(result, "left_margin", std::to_string(this->leftMargin), this->propertyStatus(comparedWith, LEFT_MARGIN_DEFINED));
      if ((this->definedFlags & RIGHT_MARGIN_DEFINED) != 0)
        this->addStyleComment(result, "right_margin", std::to_string(this->rightMargin), this->propertyStatus(comparedWith, RIGHT_MARGIN_DEFINED));
    }

    if ((this->definedFlags & HORIZONTALLY_STRETCHABLE_DEFINED) != 0)
    {
      StretchRule stretchRule = this->isHorizontallyStretchable();
      if (this->getParent() || stretchRule != StretchRule::Auto)
        this->addStyleComment(result, "horizontally_stretchable", stretchRuleToString(stretchRule), this->propertyStatus(comparedWith, HORIZONTALLY_STRETCHABLE_DEFINED));
    }

    if ((this->definedFlags & VERTICALLY_STRETCHABLE_DEFINED) != 0)
    {
      StretchRule stretchRule = this->isVerticallyStretchable();
      if (this->getParent() || stretchRule != StretchRule::Auto)
        this->addStyleComment(result, "vertically_stretchable", stretchRuleToString(stretchRule), this->propertyStatus(comparedWith, VERTICALLY_STRETCHABLE_DEFINED));
    }

    if ((this->definedFlags & HORIZONTALLY_SQUASHABLE_DEFINED) != 0)
    {
      StretchRule stretchRule = this->isHorizontallySquashable();
      if (this->getParent() || stretchRule != StretchRule::Auto)
        this->addStyleComment(result, "horizontally_squashable", stretchRuleToString(stretchRule), this->propertyStatus(comparedWith, HORIZONTALLY_SQUASHABLE_DEFINED));
    }

    if ((this->definedFlags & VERTICALLY_SQUASHABLE_DEFINED) != 0)
    {
      StretchRule stretchRule = this->isVerticallySquashable();
      if (this->getParent() || stretchRule != StretchRule::Auto)
        this->addStyleComment(result, "vertically_squashable", stretchRuleToString(stretchRule), this->propertyStatus(comparedWith, VERTICALLY_SQUASHABLE_DEFINED));
    }
  }

  const agui::LabelStyle* Style::getStyleForStatus(PropertyStatus propertyStatus)
  {
    switch (propertyStatus)
    {
      case PropertyStatus::Defines: return Gui::instance->definingStyleLabelStyle;
      case PropertyStatus::OverwritesTheSame: return Gui::instance->overwritesTheSameStyleLabelStyle;
      case PropertyStatus::Overwritten: return Gui::instance->overwritettenStyleLabelStyle;
    }
    return nullptr;
  }

  void Style::addStyleComment(VerticalFlow& result, const std::string& caption, const std::string& value, PropertyStatus propertyStatus) const
  {
    result << (agui::hFlow << agui::label("    " + caption + ":")
                           << agui::label(value, Style::getStyleForStatus(propertyStatus)));
  }

  std::string Style::getParentPathString() const
  {
    std::vector<const Style*> styles;
    for (const agui::Style* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
      styles.push_back(parent);

    std::string result = "ROOT";
    auto it = styles.rbegin();
    auto end = styles.rend();
    for (; it != end; ++it)
    {
      const Style& style = **it;
      result += " -> ";
      if (style.styleInfo)
        result += "(" + style.styleInfo->name + ")";
      else
        result += "<N.A>";
    }

    result += " -> ";
    if (this->styleInfo)
      result += " (" + this->styleInfo->name + ")";
    else
      result += "<N.A>";

    return result;
  }

  void Style::setToolTip(const std::string& tooltip)
  {
    this->tooltip.reset(new std::string(tooltip));
  }

  const std::string* Style::getToolTip() const
  {
    const Style* style = this;
    while (style && !style->tooltip)
      style = style->parent;
    return style && !style->tooltip->empty() ? style->tooltip.get() : nullptr;
  }

  StretchRule Style::isHorizontallyStretchable() const
  {
    const Style* style = this;
    while ((style->definedFlags & HORIZONTALLY_STRETCHABLE_DEFINED) == 0)
      style = style->parent;
    return StretchRule(2 * ((style->valueFlags & HORIZONTALLY_STRETCHABLE_VALUE1) != 0) + ((style->valueFlags & HORIZONTALLY_STRETCHABLE_VALUE2) != 0));
  }

  void Style::setHorizontallyStretchable(bool value)
  {
    this->setHorizontallyStretchable(value ? StretchRule::On : StretchRule::Off);
  }

  void Style::setHorizontallyStretchable(StretchRule stretchRule)
  {
    this->definedFlags |= HORIZONTALLY_STRETCHABLE_DEFINED;
    bool flag1 = uint8_t(stretchRule) >= 2;
    bool flag2 = uint8_t(stretchRule) % 2;

    if (flag1)
      this->valueFlags |= HORIZONTALLY_STRETCHABLE_VALUE1;
    else
      this->valueFlags &= ~HORIZONTALLY_STRETCHABLE_VALUE1;
    if (flag2)
      this->valueFlags |= HORIZONTALLY_STRETCHABLE_VALUE2;
    else
      this->valueFlags &= ~HORIZONTALLY_STRETCHABLE_VALUE2;

    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  StretchRule Style::isVerticallyStretchable() const
  {
    const Style* style = this;
    while ((style->definedFlags & VERTICALLY_STRETCHABLE_DEFINED) == 0)
      style = style->parent;
    return StretchRule(2 * ((style->valueFlags & VERTICALLY_STRETCHABLE_VALUE1) != 0) + ((style->valueFlags & VERTICALLY_STRETCHABLE_VALUE2) != 0));
  }

  void Style::setVerticallyStretchable(bool value)
  {
    this->setVerticallyStretchable(value ? StretchRule::On : StretchRule::Off);
  }

  void Style::setVerticallyStretchable(StretchRule stretchRule)
  {
    this->definedFlags |= VERTICALLY_STRETCHABLE_DEFINED;
    bool flag1 = uint8_t(stretchRule) >= 2;
    bool flag2 = uint8_t(stretchRule) % 2;

    if (flag1)
      this->valueFlags |= VERTICALLY_STRETCHABLE_VALUE1;
    else
      this->valueFlags &= ~VERTICALLY_STRETCHABLE_VALUE1;
    if (flag2)
      this->valueFlags |= VERTICALLY_STRETCHABLE_VALUE2;
    else
      this->valueFlags &= ~VERTICALLY_STRETCHABLE_VALUE2;

    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  agui::StretchRule Style::isHorizontallySquashable() const
  {
    const Style* style = this;
    while ((style->definedFlags & HORIZONTALLY_SQUASHABLE_DEFINED) == 0)
      style = style->parent;
    return StretchRule(2 * ((style->valueFlags & HORIZONTALLY_SQUASHABLE_VALUE1) != 0) + ((style->valueFlags & HORIZONTALLY_SQUASHABLE_VALUE2) != 0));
  }

  void Style::setHorizontallySquashable(bool horizontallySquashable)
  {
    this->setHorizontallySquashable(horizontallySquashable ? StretchRule::On : StretchRule::Off);
  }

  void Style::setHorizontallySquashable(StretchRule stretchRule)
  {
    this->definedFlags |= HORIZONTALLY_SQUASHABLE_DEFINED;
    bool flag1 = uint8_t(stretchRule) >= 2;
    bool flag2 = uint8_t(stretchRule) % 2;

    if (flag1)
      this->valueFlags |= HORIZONTALLY_SQUASHABLE_VALUE1;
    else
      this->valueFlags &= ~HORIZONTALLY_SQUASHABLE_VALUE1;
    if (flag2)
      this->valueFlags |= HORIZONTALLY_SQUASHABLE_VALUE2;
    else
      this->valueFlags &= ~HORIZONTALLY_SQUASHABLE_VALUE2;

    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  StretchRule Style::isVerticallySquashable() const
  {
    const Style* style = this;
    while ((style->definedFlags & VERTICALLY_SQUASHABLE_DEFINED) == 0)
      style = style->parent;
    return StretchRule(2 * ((style->valueFlags & VERTICALLY_SQUASHABLE_VALUE1) != 0) + ((style->valueFlags & VERTICALLY_SQUASHABLE_VALUE2) != 0));
  }

  void Style::setVerticallySquashable(bool verticallySquashable)
  {
    this->setVerticallySquashable(verticallySquashable ? StretchRule::On : StretchRule::Off);
  }

  void Style::setVerticallySquashable(StretchRule stretchRule)
  {
    this->definedFlags |= VERTICALLY_SQUASHABLE_DEFINED;
    bool flag1 = uint8_t(stretchRule) >= 2;
    bool flag2 = uint8_t(stretchRule) % 2;

    if (flag1)
      this->valueFlags |= VERTICALLY_SQUASHABLE_VALUE1;
    else
      this->valueFlags &= ~VERTICALLY_SQUASHABLE_VALUE1;
    if (flag2)
      this->valueFlags |= VERTICALLY_SQUASHABLE_VALUE2;
    else
      this->valueFlags &= ~VERTICALLY_SQUASHABLE_VALUE2;

    if (this->relatedWidget)
      this->relatedWidget->triggerResize();
  }

  agui::StretchRule stretchRuleFromString(const std::string& string)
  {
    if (string == "off")
      return StretchRule::Off;
    if (string == "on")
      return StretchRule::On;
    if (string == "auto")
      return StretchRule::Auto;
    if (string == "stretch_and_expand")
      return StretchRule::StretchAndExpand;
    throw std::runtime_error("Unknown stretch rule value: " + string);
  }

  std::string stretchRuleToString(StretchRule stretchRule)
  {
    switch (stretchRule)
    {
      case StretchRule::On: return "on";
      case StretchRule::Off: return "off";
      case StretchRule::Auto: return "auto";
      case StretchRule::StretchAndExpand: return "stretch_and_expand";
      default:;
    }
    return "invalid";
  }
}

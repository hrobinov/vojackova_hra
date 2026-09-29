#pragma once
#include "Agui/AlignmentEnum.hpp"
#include "Agui/AreaAlignmentEnum.hpp"
#include "Agui/Widget.hpp"
#include "Agui/Effect.hpp"
#include <optional>
#include <memory>
namespace agui
{
  class Font;
  class LabelStyle;
  class VerticalFlow;
  class Sound;
  class Widget;

  enum class StretchRule : uint8_t
  {
    On, // stretching is enabled
    Off, // stretching is disabled even when the container contains stretchable elements
    Auto, // stretching depends on the contents of the layout (IE. flow with stretchable element gets stretchable)
    StretchAndExpand // Scroll pane/label/checkBox/radioButton that expands its size to maximum and is stretches at the same time
  };
  StretchRule stretchRuleFromString(const std::string& string);
  std::string stretchRuleToString(StretchRule stretchRule);
  class Style;

  struct StyleInfo
  {
    std::string name;
    Style* styleThatDefinesThis = nullptr;
  };

  enum class PropertyStatus : uint8_t { Defines, Overwritten , OverwritesTheSame };
  static PropertyStatus operator||(const PropertyStatus status1, PropertyStatus status2)
  {
    return uint8_t(status1) < uint8_t(status2) ? status1 : status2;
  }

  class StylePropertyComparators
  {
  public:
    template<class PropertyType>
    static bool equal(const std::unique_ptr<PropertyType>& a, const std::unique_ptr<PropertyType>& b)
    { return *a == *b; }

    template<class PropertyType>
    static bool equal(const std::optional<PropertyType>& a, const std::optional<PropertyType>& b)
    { return *a == *b; }

    template<class PropertyType>
    // Comparing pointers directly doesn't make sense for most things - be sure you want it.
    static bool equal(const PropertyType* a, const PropertyType* b) = delete;

    static bool equal(const Sound* a, const Sound* b);
    static bool equal(const Font* a, const Font* b) { return a == b; }
  };

  class Style
  {
  public:
    explicit Style(const Style* parent = nullptr);
    explicit Style(Widget* relatedWidget, const Style* parent = nullptr, bool isLeaf = true);
    virtual ~Style() = default;
    Style(const Style&) = delete;
    Style(const Style&&) = delete;
    Style& operator=(const Style&) = delete;
    const Style* getParent() const { return this->parent; }
    virtual void setParent(const Style* parent);
    virtual void clear();
    void setParentSilent(const Style* parent); // without triggering resize
    virtual void setSize(int width, int height); // When 0 doesn't use the value.
    virtual void setNaturalSize(int width, int height); // When 0 doesn't use the value.

    bool isIgnoredBySearch() const;
    void setIgnoredBySearch(bool ignoredBySearch);
    bool canBeHiddenBySearch() const;
    void setNeverHiddenBySearch(bool neverHideBySearch);
    StretchRule isHorizontallyStretchable() const;
    void setHorizontallyStretchable(bool value = true);
    void setHorizontallyStretchable(StretchRule stretchRule);
    StretchRule isVerticallyStretchable() const;
    void setVerticallyStretchable(bool value = true);
    void setVerticallyStretchable(StretchRule stretchRule);
    StretchRule isHorizontallySquashable() const;
    void setHorizontallySquashable(bool horizontallySquashable = true);
    void setHorizontallySquashable(StretchRule stretchRule);
    StretchRule isVerticallySquashable() const;
    void setVerticallySquashable(bool verticallySquashable = true);
    void setVerticallySquashable(StretchRule stretchRule);
    HorizontalAlign getHorizontalAlign() const { return this->getFlaggedProperty(&Style::horizontalAlign, HORIZONTAL_ALIGN_DEFINED); }
    void setHorizontalAlign(HorizontalAlign horizontalAlign) { this->setFlaggedProperty(&Style::horizontalAlign, HORIZONTAL_ALIGN_DEFINED, horizontalAlign); }
    VerticalAlign getVerticalAlign() const { return this->getFlaggedProperty(&Style::verticalAlign, VERTICAL_ALIGN_DEFINED); }
    void setVerticalAlign(VerticalAlign verticalAlign) { this->setFlaggedProperty(&Style::verticalAlign, VERTICAL_ALIGN_DEFINED, verticalAlign); }
    int32_t getMinimalWidth() const { return this->getFlaggedProperty(&Style::minimalWidth, MINIMAL_WIDTH_DEFINED); }
    void setMinimalWidth(int32_t minimalWidth);
    void clearMinimalWidth();
    int32_t getNaturalWidth() const { return this->getFlaggedProperty(&Style::naturalWidth, NATURAL_WIDTH_DEFINED); }
    void setNaturalWidth(int32_t naturalWidth) { this->setFlaggedProperty(&Style::naturalWidth, NATURAL_WIDTH_DEFINED, naturalWidth); }
    int32_t getNaturalHeight() const { return this->getFlaggedProperty(&Style::naturalHeight, NATURAL_HEIGHT_DEFINED); }
    void setNaturalHeight(int32_t naturalHeight) { this->setFlaggedProperty(&Style::naturalHeight, NATURAL_HEIGHT_DEFINED, naturalHeight); }
    int32_t getMinimalHeight() const { return this->getFlaggedProperty(&Style::minimalHeight, MINIMAL_HEIGHT_DEFINED); }
    void setMinimalHeight(int32_t minimalHeight);
    void clearMinimalHeight();
    int32_t getMaximalWidth() const { return this->getFlaggedProperty(&Style::maximalWidth, MAXIMAL_WIDTH_DEFINED); }
    void setMaximalWidth(int32_t maximalWidth);
    void clearMaximalWidth();
    int32_t getMaximalHeight() const { return this->getFlaggedProperty(&Style::maximalHeight, MAXIMAL_HEIGHT_DEFINED); }
    void setMaximalHeight(int32_t maximalHeight);
    void clearMaximalHeight();
    void clearSizes();
    void setConstantWidth(int32_t width);
    void setConstantHeight(int32_t height);
    int16_t getTopPadding() const { return this->getFlaggedProperty(&Style::topPadding, TOP_PADDING_DEFINED); }
    void setTopPadding(int16_t topPadding) { this->setFlaggedProperty(&Style::topPadding, TOP_PADDING_DEFINED, topPadding); }
    int16_t getRightPadding() const { return this->getFlaggedProperty(&Style::rightPadding, RIGHT_PADDING_DEFINED); }
    void setRightPadding(int16_t rightPadding) { this->setFlaggedProperty(&Style::rightPadding, RIGHT_PADDING_DEFINED, rightPadding); }
    int16_t getBottomPadding() const { return this->getFlaggedProperty(&Style::bottomPadding, BOTTOM_PADDING_DEFINED); }
    void setBottomPadding(int16_t bottomPadding) { this->setFlaggedProperty(&Style::bottomPadding, BOTTOM_PADDING_DEFINED, bottomPadding); }
    int16_t getLeftPadding() const { return this->getFlaggedProperty(&Style::leftPadding, LEFT_PADDING_DEFINED); }
    void setLeftPadding(int16_t leftPadding) { this->setFlaggedProperty(&Style::leftPadding, LEFT_PADDING_DEFINED, leftPadding); }
    void unsetLeftPadding() { this->unsetFlaggedProperty(LEFT_PADDING_DEFINED); }
    int16_t getTopMargin() const { return this->getFlaggedProperty(&Style::topMargin, TOP_MARGIN_DEFINED); }
    void setTopMargin(int16_t topMargin) { this->setFlaggedProperty(&Style::topMargin, TOP_MARGIN_DEFINED, topMargin); }
    int16_t getRightMargin() const { return this->getFlaggedProperty(&Style::rightMargin, RIGHT_MARGIN_DEFINED); }
    void setRightMargin(int16_t rightMargin) { this->setFlaggedProperty(&Style::rightMargin, RIGHT_MARGIN_DEFINED, rightMargin); }
    int16_t getBottomMargin() const { return this->getFlaggedProperty(&Style::bottomMargin, BOTTOM_MARGIN_DEFINED); }
    void setBottomMargin(int16_t bottomMargin) { this->setFlaggedProperty(&Style::bottomMargin, BOTTOM_MARGIN_DEFINED, bottomMargin); }
    int16_t getLeftMargin() const { return this->getFlaggedProperty(&Style::leftMargin, LEFT_MARGIN_DEFINED); }
    void setLeftMargin(int16_t leftMargin) { this->setFlaggedProperty(&Style::leftMargin, LEFT_MARGIN_DEFINED, leftMargin); }
    void setPaddings(int16_t topPadding, int16_t rightPadding, int16_t bottomPadding, int16_t leftPadding);
    void setPadding(int16_t padding);
    void setMargin(int16_t margin);
    const Effect* getEffect() const { return this->getFlaggedProperty(&Style::effect, EFFECT_DEFINED); }
    void setEffect(const agui::Effect* effect) { this->setFlaggedProperty(&Style::effect, EFFECT_DEFINED, effect); }
    void unsetEffect() { this->unsetFlaggedProperty(EFFECT_DEFINED); }
    float getEffectOpacity() const { return this->getFlaggedProperty(&Style::effectOpacity, EFFECT_OPACITY_DEFINED); }
    void setEffectOpacity(float opacity) { this->setFlaggedProperty(&Style::effectOpacity, EFFECT_OPACITY_DEFINED, opacity); }
    void unsetEffectOpacity() { this->unsetFlaggedProperty(EFFECT_OPACITY_DEFINED); }
    StyleInfo& getOrCreateStyleInfo() { if (!this->styleInfo) this->styleInfo.reset(new StyleInfo()); return *this->styleInfo; }
    virtual void addChangedValues(VerticalFlow& result, const Style* comparedWith) const;
    void addStyleComment(VerticalFlow& result, const std::string& caption, const std::string& value, PropertyStatus propertyStatus) const;
    std::string getParentPathString() const;
    void setToolTip(const std::string& string);
    const std::string* getToolTip() const;

  private:
    static const LabelStyle* getStyleForStatus(PropertyStatus propertyStatus);
    const int32_t* getMaximalWidthOptional() const;
    const int32_t* getMinimalWidthOptional() const;
    const int32_t* getMaximalHeightOptional() const;
    const int32_t* getMinimalHeightOptional() const;
    PropertyStatus propertyStatus(const Style* comparedWith, uint32_t flag) const;
    bool flagValueIsTheSame(const Style* other, uint32_t flag) const;
  protected:
    template<class StyleType, class PropertyType>
    PropertyStatus propertyStatus(const Style* inputComparedWith, PropertyType StyleType::* styleProperty) const
    {
      const StyleType* comparedWith = static_cast<const StyleType*>(inputComparedWith);
      for (const StyleType* parent = static_cast<const StyleType*>(this->getParent());
           parent != nullptr;
           parent = static_cast<const StyleType*>(parent->getParent()))
        if (parent->*styleProperty)
        {
          if (StylePropertyComparators::equal(parent->*styleProperty, static_cast<const StyleType*>(this)->*styleProperty))
            return PropertyStatus::OverwritesTheSame;
          break;
        }

      for (;comparedWith != this; comparedWith = static_cast<const StyleType*>(comparedWith->getParent()))
        if (comparedWith->*styleProperty)
          return PropertyStatus::Overwritten;
      return PropertyStatus::Defines;
    }

    template<class StyleType, class PropertyType>
    const PropertyType* getProperty(std::unique_ptr<PropertyType> StyleType::*styleProperty) const
    {
      const StyleType* style = static_cast<const StyleType*>(this);
      while (!(style->*styleProperty))
        style = static_cast<const StyleType*>(style->parent);
      return (style->*styleProperty).get();
    }

    template<class StyleType, class PropertyType>
    const PropertyType* getProperty(PropertyType* StyleType::*styleProperty) const
    {
      const StyleType* style = static_cast<const StyleType*>(this);
      while (!(style->*styleProperty))
        style = static_cast<const StyleType*>(style->parent);
      return style->*styleProperty;
    }

    template<class StyleType, class PropertyType>
    const PropertyType& getProperty(std::optional<PropertyType> StyleType::*styleProperty) const
    {
      const StyleType* style = static_cast<const StyleType*>(this);
      while (!(style->*styleProperty))
        style = static_cast<const StyleType*>(style->parent);
      return *(style->*styleProperty);
    }

    template<class StyleType, class PropertyType>
    PropertyType* initProperty(std::unique_ptr<PropertyType> StyleType::*styleProperty)
    {
      StyleType* style = static_cast<StyleType*>(this);
      if (!(style->*styleProperty))
        (style->*styleProperty).reset(new PropertyType());
      return (style->*styleProperty).get();
    }

    template<class StyleType, class PropertyType>
    const PropertyType* getPropertyOptional(PropertyType* StyleType::*styleProperty) const
    {
      const StyleType* style = static_cast<const StyleType*>(this);
      while (style && !(style->*styleProperty))
        style = static_cast<const StyleType*>(style->parent);
      if (style)
        return style->*styleProperty;
      return nullptr;
    }

    template<class StyleType, class PropertyType>
    const PropertyType* getPropertyOptional(std::unique_ptr<PropertyType> StyleType::*styleProperty) const
    {
      const StyleType* style = static_cast<const StyleType*>(this);
      while (style && !(style->*styleProperty))
        style = static_cast<const StyleType*>(style->parent);
      if (style)
        return (style->*styleProperty).get();
      return nullptr;
    }

    template<class StyleType, class PropertyType>
    void setProperty(const PropertyType* StyleType::*styleProperty, const PropertyType* value)
    {
      StyleType* style = static_cast<StyleType*>(this);
      style->*styleProperty = value;
      if (this->relatedWidget)
        this->relatedWidget->triggerResize();
    }

    template<class StyleType, class PropertyType>
    void setProperty(std::optional<PropertyType> StyleType::*styleProperty, const PropertyType& value)
    {
      StyleType* style = static_cast<StyleType*>(this);
      style->*styleProperty = value;
      if (this->relatedWidget)
        this->relatedWidget->triggerResize();
    }

    template<class StyleType, class PropertyType>
    void setProperty(std::unique_ptr<PropertyType> StyleType::*styleProperty, const PropertyType& value)
    {
      StyleType* style = static_cast<StyleType*>(this);
      (style->*styleProperty).reset(new PropertyType(value));
      if (this->relatedWidget)
        this->relatedWidget->triggerResize();
    }
  private:
    template<class PropertyType>
    PropertyType getFlaggedProperty(PropertyType Style::*styleProperty, uint32_t flagValue) const
    {
      const Style* style = this;
      while ((style->definedFlags & flagValue) == 0)
        style = style->parent;
      return style->*styleProperty;
    }

    template<class PropertyType>
    void setFlaggedProperty(PropertyType Style::*styleProperty, uint32_t flagValue, PropertyType value)
    {
      this->definedFlags |= flagValue;
      this->*styleProperty = value;
      if (this->relatedWidget)
        this->relatedWidget->flagsChanged();
    }

    void unsetFlaggedProperty(uint32_t flagValue)
    {
      this->definedFlags &= ~flagValue;
    }

  protected:
    const Style* parent;
    Widget* relatedWidget = nullptr;
  public:
    std::unique_ptr<StyleInfo> styleInfo;
  private:
    constexpr static uint32_t HORIZONTAL_ALIGN_DEFINED = 1 << 0;
    constexpr static uint32_t VERTICAL_ALIGN_DEFINED = 1 << 1;
    constexpr static uint32_t MINIMAL_WIDTH_DEFINED = 1 << 2;
    constexpr static uint32_t MINIMAL_HEIGHT_DEFINED = 1 << 3;
    constexpr static uint32_t MAXIMAL_WIDTH_DEFINED = 1 << 4;
    constexpr static uint32_t MAXIMAL_HEIGHT_DEFINED = 1 << 5;
    constexpr static uint32_t TOP_PADDING_DEFINED = 1 << 6;
    constexpr static uint32_t RIGHT_PADDING_DEFINED = 1 << 7;
    constexpr static uint32_t BOTTOM_PADDING_DEFINED = 1 << 8;
    constexpr static uint32_t LEFT_PADDING_DEFINED = 1 << 9;
    constexpr static uint32_t HORIZONTALLY_STRETCHABLE_DEFINED = 1 << 10;
    constexpr static uint32_t VERTICALLY_STRETCHABLE_DEFINED = 1 << 11;
    constexpr static uint32_t HORIZONTALLY_SQUASHABLE_DEFINED = 1 << 12;
    constexpr static uint32_t VERTICALLY_SQUASHABLE_DEFINED = 1 << 13;
    constexpr static uint32_t NATURAL_WIDTH_DEFINED = 1 << 14;
    constexpr static uint32_t NATURAL_HEIGHT_DEFINED = 1 << 15;
    constexpr static uint32_t TOP_MARGIN_DEFINED = 1 << 16;
    constexpr static uint32_t RIGHT_MARGIN_DEFINED = 1 << 17;
    constexpr static uint32_t BOTTOM_MARGIN_DEFINED = 1 << 18;
    constexpr static uint32_t LEFT_MARGIN_DEFINED = 1 << 19;
    constexpr static uint32_t EFFECT_DEFINED = 1 << 20;
    constexpr static uint32_t EFFECT_OPACITY_DEFINED = 1 << 21;
    constexpr static uint32_t IGNORED_BY_SEARCH_DEFINED = 1 << 22;
    constexpr static uint32_t NEVER_HIDE_BY_SEARCH_DEFINED = 1 << 23;

    constexpr static uint16_t HORIZONTALLY_STRETCHABLE_VALUE1 = 1 << 0; // first bit definingStretchRule
    constexpr static uint16_t HORIZONTALLY_STRETCHABLE_VALUE2 = 1 << 1; // second bit definingStretchRule
    constexpr static uint16_t VERTICALLY_STRETCHABLE_VALUE1 = 1 << 2; // first bit definingStretchRule
    constexpr static uint16_t VERTICALLY_STRETCHABLE_VALUE2 = 1 << 3; // second bit definingStretchRule
    constexpr static uint16_t VERTICALLY_SQUASHABLE_VALUE1 = 1 << 4;
    constexpr static uint16_t VERTICALLY_SQUASHABLE_VALUE2 = 1 << 5;
    constexpr static uint16_t HORIZONTALLY_SQUASHABLE_VALUE1 = 1 << 6;
    constexpr static uint16_t HORIZONTALLY_SQUASHABLE_VALUE2 = 1 << 7;
    constexpr static uint16_t IGNORED_BY_SEARCH_VALUE = 1 << 8;
    constexpr static uint16_t NEVER_HIDE_BY_SEARCH_VALUE = 1 << 9;
    uint32_t definedFlags = 0;
    uint16_t valueFlags = 0;
    /** Align of the internal content, for simple elements like button, label,
     * this will take effect only when the size of the element is bigger than needed.
     * For layouts this affects the default align of the single elements. */
    HorizontalAlign horizontalAlign = HorizontalAlign::Left;
    VerticalAlign verticalAlign = VerticalAlign::Top;
    int32_t minimalWidth = 0;
    int32_t minimalHeight = 0;
    int32_t naturalWidth = 0;
    int32_t naturalHeight = 0;
    int32_t maximalWidth = 0;
    int32_t maximalHeight = 0;
    int16_t topPadding = 0;
    int16_t rightPadding = 0;
    int16_t bottomPadding = 0;
    int16_t leftPadding = 0;
    int16_t topMargin = 0;
    int16_t rightMargin = 0;
    int16_t bottomMargin = 0;
    int16_t leftMargin = 0;
    const Effect* effect = nullptr;
    float effectOpacity = 1;
    std::unique_ptr<std::string> tooltip;
  };
}

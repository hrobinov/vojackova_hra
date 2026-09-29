#pragma once
#include "Agui/Widget/CheckBoxStyle.hpp"
#include "Agui/Widget/ToggleButton.hpp"
#include "Agui/ResizableText.hpp"

namespace agui
{
  /** CheckBox that can be CHECKED, UNCHECKED, or INTERMEDIATE. */
  class CheckBox : public ToggleButton
  {
    using super = ToggleButton;
  public:
    using StyleType = CheckBoxStyle;

    CheckBox(const StyleType* parentStyle = &CheckBox::defaultStyle);
    CheckBox(std::string&& text, const StyleType* parentStyle = &CheckBox::defaultStyle);
    virtual ~CheckBox(void);

  protected:
    virtual void drawItem(const PaintEvent& paintEvent, const  Rectangle& rectangle) const override;

    virtual const ElementImageSet* getBackgroundGraphicalSet() const override;
    const Image* getMark() const;

    virtual Dimension getDimension() const override;

    // for passing style from children
    virtual const Font* getFont() const override { return this->style.getFont();  }
    virtual const Color& getFontColor() const override;
    virtual HorizontalAlign getHorizontalAlign() const override { return this->style.getHorizontalAlign(); }
    virtual VerticalAlign getVerticalAlign() const override { return this->style.getVerticalAlign(); }
    virtual uint32_t getTextPadding() const override { return this->style.getTextPadding(); }
    virtual bool genericSearch(const LowercaseString& filter) override;
  public:
    virtual Style* getStyle() override { return &this->style; }

    static StyleType defaultStyle;
    StyleType style;
  };
}

#pragma  once
#include "Agui/Widget/RadioButtonStyle.hpp"
#include "Agui/Widget/ToggleButton.hpp"

namespace agui
{
  class RadioButton : public ToggleButton
  {
    using super = ToggleButton;
  public:
    RadioButton(const RadioButtonStyle* parentStyle = &RadioButton::defaultStyle);
    RadioButton(std::string&& text, const RadioButtonStyle* parentStyle = &RadioButton::defaultStyle);
    virtual ~RadioButton() = default;

    virtual void nextCheckState() override; // Internally changes to the next logical checked state.
  protected:

    virtual void drawItem(const PaintEvent&, const  Rectangle&) const override {}; // all is done by bg here
    virtual const ElementImageSet* getBackgroundGraphicalSet() const override;

    virtual Dimension getDimension() const override;

    virtual const Font* getFont() const override { return this->style.getFont(); }
    virtual const Color& getFontColor() const override;
    virtual HorizontalAlign getHorizontalAlign() const override { return this->style.getHorizontalAlign(); }
    virtual VerticalAlign getVerticalAlign() const override { return this->style.getVerticalAlign(); }
    virtual uint32_t getTextPadding() const override { return this->style.getTextPadding(); }
    virtual bool genericSearch(const LowercaseString& filter) override;
  public:
    virtual Style* getStyle() override { return &this->style; }

    static RadioButtonStyle defaultStyle;

    RadioButtonStyle style;
  };
}

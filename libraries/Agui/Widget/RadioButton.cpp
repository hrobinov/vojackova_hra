#include "Agui/Widget/RadioButton.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Font.hpp"
#include "Agui/ResizableText.hpp"
#include <Agui/StringMatcher.hpp>
#include <algorithm>
#include <stdexcept>

namespace agui
{
  RadioButtonStyle RadioButton::defaultStyle;

  RadioButton::RadioButton(const RadioButtonStyle* parentStyle)
    : style(this, parentStyle)
  {}

  RadioButton::RadioButton(std::string&& text, const RadioButtonStyle* parentStyle)
    : ToggleButton(std::move(text))
    , style(this, parentStyle)
  {}

  void RadioButton::nextCheckState()
  {
    if (!this->isEnabled())
      return;
    if (!this->isChecked()) // if it's checked, we don't want to change it
    {
      this->changeCheckedState(CheckedState::CHECKED);
      this->dispatchCheck();
    }
    this->dispatchCheckChange();
  }

  const ElementImageSet* RadioButton::getBackgroundGraphicalSet() const
  {
    if (!this->isEnabled())
      return this->style.getDisabledGraphicalSet();

    switch (this->getClickStateForRendering())
    {
      case ClickState::DEFAULT: return this->isChecked() ? this->style.getSelectedGraphicalSet() : this->style.getDefaultGraphicalSet();
      case ClickState::HOVERED: return this->isChecked() ? this->style.getSelectedHoveredGraphicalSet() : this->style.getHoveredGraphicalSet();
      case ClickState::CLICKED: return this->isChecked() ? this->style.getSelectedClickedGraphicalSet() : this->style.getClickedGraphicalSet();
    }

    abort();
  }

  Dimension RadioButton::getDimension() const
  {
    return Dimension(16, 16); // should be in style
  }

  const agui::Color& RadioButton::getFontColor() const
  {
    return this->isEnabled() ? this->style.getFontColor() : this->style.getDisabledFontColor();
  }

  bool RadioButton::genericSearch(const LowercaseString& filter)
  {
    return this->setShownBySearch(StringMatcher::matchesSearchPattern(this->getText(), filter) != StringMatcherResult::NoMatch);
  }
}

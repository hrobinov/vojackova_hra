#include "Agui/Widget/CheckBox.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Font.hpp"
#include "Agui/Image.hpp"
#include <Agui/StringMatcher.hpp>
#include <algorithm>

namespace agui
{
  CheckBox::StyleType CheckBox::defaultStyle;

  CheckBox::CheckBox(const StyleType* parentStyle)
    : style(this, parentStyle)
  {}

  CheckBox::CheckBox(std::string&& text, const StyleType* parentStyle)
    : ToggleButton(std::move(text))
    , style(this, parentStyle)
  {}

  CheckBox::~CheckBox(void)
  {}

  void CheckBox::drawItem(const PaintEvent& paintEvent, const Rectangle& rectangle) const
  {
    if (const Image* mark = this->getMark())
      paintEvent.graphics()->drawScaledImage(mark,
                                             Point(rectangle.getLeft(), rectangle.getTop()),
                                             Dimension(mark->getWidth() * Gui::instance->scale, mark->getHeight() * Gui::instance->scale));
  }

  const ElementImageSet* CheckBox::getBackgroundGraphicalSet() const
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

  const Image* CheckBox::getMark() const
  {
    switch (this->getCheckedState())
    {
      case CheckedState::CHECKED:
        if (this->isEnabled())
          return this->style.getCheckmark();
        return this->style.getDisabledCheckmark();
      case CheckedState::INTERMEDIATE:
        return this->style.getIntermediateMark();
      case CheckedState::UNCHECKED: break;
    }

    return nullptr;
  }

  Dimension CheckBox::getDimension() const
  {
    if (const Image* mark = this->getMark())
      return mark->getDimension();
    return this->style.getCheckmark()->getDimension();
  }

  const agui::Color& CheckBox::getFontColor() const
  {
    return this->isEnabled() ? this->style.getFontColor() : this->style.getDisabledFontColor();
  }

  bool CheckBox::genericSearch(const LowercaseString& filter)
  {
    return this->setShownBySearch(StringMatcher::matchesSearchPattern(this->getText(), filter) != StringMatcherResult::NoMatch);
  }
}

#include <ui/Resettable.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/ImageWidget.hpp>

#include <algorithm>
#include <string>

namespace ui {

namespace {

// What the reset button puts back: the settings a first run starts with.
const Settings DEFAULTS{};

// tool_button_red, and the icon on it.
constexpr int BUTTON_PX = 28;
constexpr int ICON_PX   = 16;

// locale core.cfg: reset-to-defaults and reset-to-defaults-disabled.
std::string ResetTip(int count)
{
  if (count == 0) return "Všechna nastavení mají výchozí hodnoty.";
  return count == 1 ? "Vrátit 1 nastavení na výchozí hodnotu"
                    : "Vrátit " + std::to_string(count) + " nastavení na výchozí hodnoty";
}

}  // namespace

Resettable::Resettable(Theme& theme, std::function<void()> onReset)
    : theme(theme)
{
  // tool_button_red with Factorio's reset arrow, dark, or white while it is
  // disabled.
  this->reset = &make<agui::Button>(&theme.redToolButton);
  this->reset->setFocusable(false);
  this->icon = &make<agui::ImageWidget>(theme.resetIcon(true));
  this->icon->scaleToKeepTheRatio = true;
  this->icon->setIgnoredByInteraction(true);
  this->icon->style.setMinimalWidth(ICON_PX);
  this->icon->style.setMaximalWidth(ICON_PX);
  this->icon->style.setMinimalHeight(ICON_PX);
  this->icon->style.setMaximalHeight(ICON_PX);
  *this->reset << *this->icon;
  this->lightWhileHovered(*this->reset, []() -> const Settings& { return DEFAULTS; });
  this->reset->onClick(this, [this, onReset = std::move(onReset)](const agui::MouseEvent& event) {
    // Before the reset: after it, nothing says which were lit.
    this->unhighlight(this->reset, event);
    onReset();
  });
}

void Resettable::track(agui::Widget& widget, std::function<bool(const Settings&)> differs)
{
  this->tracked.push_back({ &widget, std::move(differs) });
}

void Resettable::lightWhileHovered(agui::Button& button, std::function<const Settings&()> reference)
{
  button.onMouseEnter(this, [this, &button, reference](const agui::MouseEvent& event) {
    this->highlight(&button, reference(), event);
  });
  button.onMouseLeave(this, [this, &button](const agui::MouseEvent& event) { this->unhighlight(&button, event); });
}

void Resettable::highlight(const agui::Widget* source, const Settings& reference, const agui::MouseEvent& event)
{
  this->unhighlight(this->litBy, event);
  this->litBy = source;
  for (const Setting& s : this->tracked) {
    if (!s.differs(reference)) continue;
    s.widget->mouseEnter(event);
    this->lit.push_back(s.widget);
  }
}

void Resettable::unhighlight(const agui::Widget* source, const agui::MouseEvent& event)
{
  if (source != this->litBy) return;
  for (agui::Widget* widget : this->lit) widget->mouseLeave(event);
  this->lit.clear();
  this->litBy = nullptr;
}

void Resettable::changed()
{
  // Nothing to reset when everything is already the default.
  const int count = int(std::count_if(this->tracked.begin(), this->tracked.end(),
                                      [](const Setting& s) { return s.differs(DEFAULTS); }));
  if (this->reset->isEnabled() != (count != 0)) {
    this->reset->setEnabled(count != 0);
    this->icon->setImage(this->theme.resetIcon(count != 0));
  }
  this->reset->setToolTip(ResetTip(count));

  // The icon in the middle of the button. A button's children sit inside its
  // padding, border and all, so that comes off.
  this->icon->setLocation((BUTTON_PX - ICON_PX) / 2 - this->reset->getLeftPadding(),
                          (BUTTON_PX - ICON_PX) / 2 - this->reset->getTopPadding());
}

}  // namespace ui

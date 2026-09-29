#include <ui/SearchBar.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/ImageWidget.hpp>
#include <Agui/Widget/TextField.hpp>
#include <Agui/Widget/Window.hpp>

#include <algorithm>

namespace ui {

namespace {

// frame_action_button. The icon fills what is inside its border.
constexpr int BUTTON_PX = 24;

}  // namespace

SearchBar::SearchBar(Theme& theme, agui::Window& owner, OnChange onChange)
    : agui::Button(&theme.frameActionButton)
    , onChange(std::move(onChange))
{
  this->setFocusable(false);
  this->setToggleButton(true);
  this->icon = &make<agui::ImageWidget>(theme.searchIcon(), &theme.frameActionIcon);
  this->icon->scaleToKeepTheRatio = true;
  this->icon->setIgnoredByInteraction(true);
  this->icon->style.setMinimalWidth(BUTTON_PX);
  this->icon->style.setMaximalWidth(BUTTON_PX);
  this->icon->style.setMinimalHeight(BUTTON_PX);
  this->icon->style.setMaximalHeight(BUTTON_PX);
  *this << *this->icon;

  // The field, in a strip of its own laid over the title bar -- a child of
  // the window itself, not of its contents, so it is placed rather than
  // laid out (see paintComponent).
  this->popup = &make<agui::Window>(agui::GuiDirection::Horizontal, &theme.searchPopupFrame);
  this->field = &make<agui::TextField>(&theme.searchPopupField);
  *this->popup << *this->field;
  owner.Widget::add(this->popup);
  owner.addToHeaderFlow(this);

  this->field->onTextEdit(this, [this] { this->changed(); });
  // Enter is done with searching, but leaves what was found.
  this->field->onConfirm(this, [this] { this->field->clearFocus(); });
  this->onClick(this, [this] {
    const bool open = !this->isOpen();
    this->show(open);
  });
  this->show(false);
  this->setShortcut({});
}

void SearchBar::show(bool open)
{
  this->setToggleState(open);
  this->popup->setVisible(open);
  if (open) {
    this->popup->positionToWidget(this, agui::Window::AnchorType::CenterLeftToLeft);
    this->field->focus();
    this->field->selectAll();
  } else if (!this->field->getText().empty()) {
    this->field->setTextAndCallOnTextEdited(std::string());
  }
}

void SearchBar::changed()
{
  const std::string& typed = this->field->getText();
  if (this->kept && typed.empty() == this->pinned) {
    agui::Style& keptStyle = *this->kept->getStyle();
    this->pinned = !typed.empty();
    keptStyle.setMinimalWidth(this->pinned ? int16_t(this->kept->getWidth()) : 0);
    keptStyle.setMinimalHeight(this->pinned ? int16_t(this->kept->getHeight()) : 0);
  }
  this->onChange(typed);
}

void SearchBar::focusSearch()
{
  this->show(true);
}

bool SearchBar::clearAndHide()
{
  if (!this->isOpen()) return false;
  this->show(false);
  return true;
}

bool SearchBar::isOpen() const
{
  return this->popup->isVisible();
}

bool SearchBar::isSearchFocused() const
{
  return this->field->isFocused();
}

void SearchBar::setShortcut(const std::string& keys)
{
  // locale core.cfg: gui.search-with-focus.
  this->setToolTip(keys.empty() ? std::string("Hledat") : "Hledat (" + ShortcutText(keys) + ")");
}

void SearchBar::paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition)
{
  // Kept beside the button as the window moves.
  if (this->popup->isVisible()) this->popup->positionToWidget(this, agui::Window::AnchorType::CenterLeftToLeft);
  // As Factorio's IconButton: the icon fills the button's content -- inside
  // its padding, which counts the border of what it is drawn with, so 16 of
  // the 24 -- square, and centred in it.
  const int side = std::min(this->getContentWidth(), this->getContentHeight());
  if (side > 0 && this->icon->getWidth() != side) {
    this->icon->style.setMinimalWidth(side);
    this->icon->style.setMaximalWidth(side);
    this->icon->style.setMinimalHeight(side);
    this->icon->style.setMaximalHeight(side);
    this->icon->setSize(side, side, agui::SetSizeInfo());
  }
  const int x = (this->getContentWidth() - side) / 2;
  const int y = (this->getContentHeight() - side) / 2;
  if (this->icon->getLocation().x != x || this->icon->getLocation().y != y) this->icon->setLocation(x, y);
  // Black while hovered or while down: Factorio's IconButton rule, worked out
  // afresh every time. (The button only tells the icon when the mouse comes
  // and goes, not when the search closes by Esc, which would leave it black.)
  this->icon->setParentHovered(this->getClickStateForRendering() != ClickState::DEFAULT || this->isToggled());
  agui::Button::paintComponent(paintEvent, absolutePosition);
}

}  // namespace ui

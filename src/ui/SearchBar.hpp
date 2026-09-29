// The search of a settings window, as Factorio's SearchBar: a button with a
// magnifying glass in the window's title bar that opens a text field just
// left of it, over the title. Whatever is typed there is handed on as it
// changes; closing the field (the button again, or Esc) clears it.

#pragma once

#include <Agui/Widget/Button.hpp>

#include <functional>
#include <string>

namespace agui {
class ImageWidget;
class TextField;
class Window;
}  // namespace agui

namespace ui {

class Theme;

class SearchBar : public agui::Button {
public:
  using OnChange = std::function<void(const std::string& text)>;

  // Puts itself in `owner`'s title bar.
  SearchBar(Theme& theme, agui::Window& owner, OnChange onChange);

  // Opens the field with the caret in it, the text there selected.
  void focusSearch();
  // Closes the field and clears it; false if it wasn't open.
  bool clearAndHide();
  bool isOpen() const;
  bool isSearchFocused() const;

  // While something is searched for, `widget` keeps the size it had: as
  // Factorio's windows, which don't shrink round what is found.
  void keepSizeOf(agui::Widget& widget) { this->kept = &widget; }

  // The keys that focus the search, for the tooltip; empty for none.
  void setShortcut(const std::string& keys);

  void paintComponent(const agui::PaintEvent& paintEvent, const agui::Point& absolutePosition) override;

private:
  void show(bool open);
  void changed();

  OnChange           onChange;
  agui::ImageWidget* icon  = nullptr;
  agui::Window*      popup = nullptr;  // belongs to the owner, like the button
  agui::TextField*   field = nullptr;
  agui::Widget*      kept  = nullptr;
  bool               pinned = false;
};

}  // namespace ui

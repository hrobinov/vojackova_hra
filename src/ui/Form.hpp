// The bits of Agui boilerplate every page here repeats: making a widget that
// belongs to the tree it is added to, and laying settings out as a column of
// named rows.

#pragma once

#include <Agui/Widget.hpp>
#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/EmptyWidgetStyle.hpp>
#include <Agui/Widget/Filler.hpp>
#include <Agui/Widget/HorizontalFlow.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/ToggleButton.hpp>
#include <Agui/Widget/VerticalFlow.hpp>
#include <Agui/Widget/Window.hpp>

#include <functional>
#include <string>
#include <utility>

namespace ui {

// Every widget on a page is created with `new` and hold()-ed: it belongs to the
// parent it is added to, and is deleted along with it.
template<class W, class... Args>
W& make(Args&&... args)
{
  return agui::hold(*new W(std::forward<Args>(args)...));
}

// A column of rows.
inline agui::VerticalFlow& column(int spacing = 8)
{
  agui::VerticalFlow& flow = make<agui::VerticalFlow>();
  flow.style.setVerticalSpacing(spacing);
  return flow;
}

// A row of widgets, centred on each other.
inline agui::HorizontalFlow& row(int spacing = 12)
{
  agui::HorizontalFlow& flow = make<agui::HorizontalFlow>();
  flow.style.setHorizontalSpacing(spacing);
  flow.style.setVerticalAlign(agui::VerticalAlign::Center);
  return flow;
}

// A setting's name and its control, side by side. The name column is a fixed
// width, so the controls of a page line up under each other. The name of a
// checkbox is part of it, as in a browser: clicking the name toggles the box,
// hovering it lights the box up and shows the box's tooltip.
inline agui::HorizontalFlow& namedRow(const char* name, agui::Widget& control, int nameWidth = 150)
{
  agui::HorizontalFlow& flow = row();
  agui::Label& label = agui::label(name);
  label.style.setMinimalWidth(nameWidth);
  if (dynamic_cast<agui::ToggleButton*>(&control)) label.sharesTooltipWith(&control);
  flow << label << control;
  return flow;
}

// A button for the row of them along the bottom of a page. The menu's big
// button styles stretch to fill the column they were made for, which in a row
// has them swallow the gap a pusher is meant to open, so the width is pinned.
inline agui::Button& footerButton(const char* text, agui::Widget* holder,
                                  std::function<void()> callback,
                                  const agui::ButtonStyle* style, int width)
{
  agui::Button& button = agui::button(std::string(text), holder, std::move(callback), style);
  button.style.setHorizontallyStretchable(false);
  button.style.setMinimalWidth(width);
  return button;
}

// The ridged strip that fills the gap in a row of dialog buttons. It is also
// the window's second drag handle: Factorio's dialogs are moved by it as much
// as by their title bar, which is why it is a Filler rather than a pusher.
inline agui::Filler& dragHandle(const agui::EmptyWidgetStyle* style, agui::Window* dragTarget)
{
  agui::Filler& filler = make<agui::Filler>(style);
  filler.setDragTarget(dragTarget);
  return filler;
}

}  // namespace ui

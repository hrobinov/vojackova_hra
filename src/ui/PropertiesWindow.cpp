#include <ui/PropertiesWindow.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>

#include <utility>

namespace ui {

namespace {

// Wide enough for the longest title.
constexpr int PANEL_W = 240;

}  // namespace

PropertiesWindow::PropertiesWindow(Theme& theme, const std::string& title)
    : agui::Window(agui::GuiDirection::Vertical, title)
{
  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel.style.setMinimalWidth(PANEL_W);
  for (auto [name, value] : { std::pair{ "Akce", &this->actions }, std::pair{ "Životy", &this->lives },
                              std::pair{ "Zranění", &this->wounds } }) {
    *value = &agui::label("");
    agui::HorizontalFlow& line = row();
    line.style.setHorizontallyStretchable(true);
    line << agui::label(name) << agui::pusher << **value;
    panel << line;
  }
  *this << panel;
}

void PropertiesWindow::show(const Properties& properties)
{
  this->actions->setText(std::to_string(properties.actions));
  this->lives->setText(std::to_string(properties.lives));
  this->wounds->setText(std::to_string(properties.wounds));
}

}  // namespace ui

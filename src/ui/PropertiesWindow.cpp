#include <ui/PropertiesWindow.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>

#include <utility>

namespace ui {

PropertiesWindow::PropertiesWindow(Theme& theme, const std::string& title, bool armed)
    : agui::Window(agui::GuiDirection::Vertical, title)
{
  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel.style.setMinimalWidth(PANEL_W);
  for (auto [name, value] : { std::pair{ "Akce", &this->actions }, std::pair{ "Životy", &this->lives },
                              std::pair{ "Zranění", &this->wounds }, std::pair{ "Dostřel", &this->range },
                              std::pair{ "Rakety", &this->rockets }, std::pair{ "Brokovnice", &this->shells } }) {
    *value = &agui::label("");
    agui::HorizontalFlow& line = row();
    line.style.setHorizontallyStretchable(true);
    line << agui::label(name) << agui::pusher << **value;
    if (value == &this->range) line.setVisible(armed);
    if (value == &this->rockets) this->rocketsRow = &line;
    if (value == &this->shells) this->shellsRow = &line;
    panel << line;
  }
  this->armed = armed;
  *this << panel;
}

void PropertiesWindow::show(const Properties& properties)
{
  this->actions->setText(std::to_string(properties.actions));
  this->lives->setText(std::to_string(properties.lives));
  this->wounds->setText(std::to_string(properties.wounds));
  this->range->setText(std::to_string(properties.range));
  this->rockets->setText(std::to_string(properties.rockets));
  this->shells->setText(std::to_string(properties.shells));
  this->rocketsRow->setVisible(this->armed && properties.rockets > 0);
  this->shellsRow->setVisible(this->armed && properties.shells > 0);
}

}  // namespace ui

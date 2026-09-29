#include <ui/AboutPage.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>

namespace ui {

namespace {

// Wide enough that the page doesn't look squeezed round its one line.
constexpr int PANEL_W = 400;

}  // namespace

AboutPage::AboutPage(Theme& theme, std::function<void()> onBack)
    : window(agui::GuiDirection::Vertical, "O hře")
{
  this->window.setDragTarget(&this->window);

  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel.style.setMinimalWidth(PANEL_W);
  panel << agui::label("Ahoj, já jsem Robin.");
  this->window << panel;

  // As a Factorio dialog: Back on the left, the ridged strip filling the rest.
  agui::HorizontalFlow& footer = row(8);
  footer.style.setTopPadding(8);
  footer.style.setHorizontallyStretchable(true);
  footer << agui::button("Zpět", &this->window, std::move(onBack), &theme.backButton);
  footer << dragHandle(&theme.draggableSpace, &this->window);
  this->window << footer;
}

}  // namespace ui

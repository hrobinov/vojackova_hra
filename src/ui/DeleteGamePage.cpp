#include <ui/DeleteGamePage.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>

namespace ui {

namespace {

constexpr int PANEL_W = 360;

}  // namespace

DeleteGamePage::DeleteGamePage(Theme& theme, std::function<void()> onDelete, std::function<void()> onCancel)
    : window(agui::GuiDirection::Vertical, "Smazat hru")
{
  this->window.setDragTarget(&this->window);

  this->question = &agui::label("");
  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel.style.setMinimalWidth(PANEL_W);
  panel << *this->question << agui::label("Smazaná hra už nejde vrátit.", &theme.dimLabel);
  this->window << panel;

  // As Factorio's confirmations: Cancel on the left, the red button that
  // deletes on the right.
  agui::HorizontalFlow& footer = row(8);
  footer.style.setTopPadding(8);
  footer.style.setHorizontallyStretchable(true);
  footer << agui::button("Zrušit", &this->window, std::move(onCancel), &theme.backButton);
  footer << dragHandle(&theme.draggableSpace, &this->window);
  footer << footerButton("Smazat hru", &this->window, std::move(onDelete), &theme.redForwardButton, 160);
  this->window << footer;
}

void DeleteGamePage::setGame(const std::string& name)
{
  this->question->setText("Opravdu chceš smazat hru „" + name + "“?");
}

}  // namespace ui

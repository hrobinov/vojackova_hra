#include <ui/LoadGamePage.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/ImageWidget.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/VerticalScrollPane.hpp>

namespace ui {

namespace {

constexpr int LIST_W = 440;
constexpr int LIST_H = 320;  // more saves than fit scroll

// tool_button_red, and the trash can on it, as the settings' reset button.
constexpr int BUTTON_PX = 28;
constexpr int ICON_PX   = 16;

}  // namespace

LoadGamePage::LoadGamePage(Theme& theme, std::function<void(const std::filesystem::path&)> onChosen,
                           std::function<void(const SavedGame&)> onDelete, std::function<void()> onBack)
    : theme(theme)
    , window(agui::GuiDirection::Vertical, "Načíst hru")
    , onChosen(std::move(onChosen))
    , onDelete(std::move(onDelete))
{
  this->window.setDragTarget(&this->window);

  this->rows = &column(4);
  this->rows->style.setMinimalWidth(LIST_W);
  agui::VerticalScrollPane& scroll = make<agui::VerticalScrollPane>();
  scroll.style.setMaximalHeight(LIST_H);
  scroll << *this->rows;

  this->empty = &agui::label("Zatím tu nejsou žádné uložené hry.", &theme.dimLabel);
  this->empty->style.setMinimalWidth(LIST_W);

  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel << scroll << *this->empty;
  this->window << panel;

  agui::HorizontalFlow& footer = row(8);
  footer.style.setTopPadding(8);
  footer.style.setHorizontallyStretchable(true);
  footer << agui::button("Zpět", &this->window, std::move(onBack), &theme.backButton);
  footer << dragHandle(&theme.draggableSpace, &this->window);
  this->window << footer;
}

void LoadGamePage::open()
{
  this->saves = ListSavedGames();
  this->rows->clear();
  for (size_t i = 0; i < this->saves.size(); ++i) {
    const SavedGame& save = this->saves[i];

    agui::Button& load = agui::button(std::string(save.name), &this->window, [this, i] { this->onChosen(this->saves[i].path); });
    load.style.setHorizontallyStretchable(true);

    agui::Button& remove = make<agui::Button>(&this->theme.redToolButton);
    remove.setFocusable(false);
    remove.setToolTip("Smazat hru");
    agui::ImageWidget& icon = make<agui::ImageWidget>(this->theme.trashIcon());
    icon.scaleToKeepTheRatio = true;
    icon.setIgnoredByInteraction(true);
    icon.style.setMinimalWidth(ICON_PX);
    icon.style.setMaximalWidth(ICON_PX);
    icon.style.setMinimalHeight(ICON_PX);
    icon.style.setMaximalHeight(ICON_PX);
    remove << icon;
    // In the middle of the button; its children sit inside its padding.
    icon.setLocation((BUTTON_PX - ICON_PX) / 2 - remove.getLeftPadding(), (BUTTON_PX - ICON_PX) / 2 - remove.getTopPadding());
    remove.onClick(this, [this, i](const agui::MouseEvent&) { this->onDelete(this->saves[i]); });

    agui::HorizontalFlow& line = row(8);
    line.style.setHorizontallyStretchable(true);
    line << load << agui::label(SaveTimeText(save.saved), &this->theme.dimLabel) << remove;
    *this->rows << line;
  }
  this->rows->setVisible(!this->saves.empty());
  this->empty->setVisible(this->saves.empty());
}

}  // namespace ui

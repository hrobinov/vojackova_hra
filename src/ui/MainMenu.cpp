#include <ui/MainMenu.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Gui.hpp>
#include <Agui/Widget/Button.hpp>

namespace ui {

MainMenu::MainMenu(agui::Gui& gui, Theme& theme)
    : gui(gui)
    , window(agui::GuiDirection::Vertical, &theme.menuFrame, agui::Window::HeightRule::MaxScreenHeightWithExtraSpace,
             "Vojáčková hra")
    , about(theme, [this] { this->back(); })
{
  agui::VerticalFlow& buttons = column(8);
  buttons << agui::button("Nová hra", &this->window, [this] { this->pending = Action::NewGame; }, &theme.continueButton);
  buttons << agui::button("Načíst hru", &this->window, [this] { this->pending = Action::LoadGame; }, &theme.menuButton);
  buttons << agui::button("O hře", &this->window, [this] { this->open(Page::About); }, &theme.menuButton);
  buttons << agui::button("Konec", &this->window, [this] { this->pending = Action::Quit; }, &theme.menuButton);
  this->window << buttons;

  gui.add(&this->window);
  gui.add(&this->about.root());
  this->open(Page::Menu);
}

MainMenu::~MainMenu()
{
  this->gui.remove(&this->about.root());
  this->gui.remove(&this->window);
}

void MainMenu::open(Page p)
{
  this->page     = p;
  this->recentre = true;
  this->window.setVisible(p == Page::Menu);
  this->about.root().setVisible(p == Page::About);
  this->gui.clearFocus();
}

void MainMenu::back()
{
  if (this->page != Page::Menu) this->open(Page::Menu);
}

agui::Window& MainMenu::current()
{
  return this->page == Page::About ? this->about.root() : this->window;
}

void MainMenu::layout(int screenWidth, int screenHeight)
{
  if (screenWidth != this->lastScreenWidth || screenHeight != this->lastScreenHeight) {
    this->lastScreenWidth  = screenWidth;
    this->lastScreenHeight = screenHeight;
    this->recentre = true;
  }

  // Centred when it opens or the screen changes size; in between a page can
  // be dragged by its title bar, but is kept on screen.
  agui::Window& shown = this->current();
  if (this->recentre && shown.getWidth() > 0) {  // not before its first layout
    shown.setLocation((screenWidth - shown.getWidth()) / 2, (screenHeight - shown.getHeight()) / 2);
    this->recentre = false;
  }
  shown.ensureWholeWindowIsOnScreen();
}

MainMenu::Action MainMenu::takeAction()
{
  const Action a = this->pending;
  this->pending = Action::None;
  return a;
}

}  // namespace ui

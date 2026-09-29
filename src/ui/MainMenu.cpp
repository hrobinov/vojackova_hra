#include <ui/MainMenu.hpp>

#include <game/BoardView.hpp>

#include <ui/Form.hpp>
#include <ui/SearchBar.hpp>
#include <ui/Theme.hpp>

#include <Agui/Gui.hpp>
#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/TextField.hpp>

namespace ui {

namespace {

// How far the game menu's windows and the round's lives and earnings are
// from the corner, and the windows apart.
constexpr int CORNER_GAP = 16;
// How wide the secret password's field is.
constexpr int PASSWORD_W = 140;

}  // namespace

MainMenu::MainMenu(agui::Gui& gui, Theme& theme, const ::Settings& live)
    : settings(theme, live,
               [this] {
                 this->pending = Action::SaveSettings;
                 this->open(Page::Menu);
               },
               [this] { this->open(Page::Menu); })
    , gui(gui)
    , window(agui::GuiDirection::Vertical, &theme.menuFrame, agui::Window::HeightRule::MaxScreenHeightWithExtraSpace,
             "Vojáčková hra")
    , newGame(theme, [this] { this->pending = Action::NewGame; }, [this] { this->open(Page::Menu); })
    , loadGame(theme,
               [this](const std::filesystem::path& path) {
                 this->chosen  = path;
                 this->pending = Action::LoadGame;
               },
               [this](const SavedGame& save) {
                 this->toDelete = save.path;
                 this->deleteGame.setGame(save.name);
                 this->open(Page::DeleteGame);
               },
               [this] { this->open(Page::Menu); })
    , deleteGame(theme,
                 [this] {
                   DeleteSave(this->toDelete);
                   this->loadGame.open();
                   this->open(Page::LoadGame);
                 },
                 [this] { this->open(Page::LoadGame); })
    , about(theme, [this] { this->open(Page::Menu); })
    , game(agui::GuiDirection::Vertical, &theme.menuFrame, agui::Window::HeightRule::MaxScreenHeightWithExtraSpace)
    , figure(theme, "Základní vlastnosti panáčka")
    , zombie(theme, "Základní vlastnosti bílého zombíka")
    , round(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding)
    , money(agui::GuiDirection::Vertical, "Peníze")
    , gamePause(agui::GuiDirection::Vertical, &theme.menuFrame, agui::Window::HeightRule::MaxScreenHeightWithExtraSpace)
    , gobanPause(agui::GuiDirection::Vertical, &theme.menuFrame, agui::Window::HeightRule::MaxScreenHeightWithExtraSpace)
{
  agui::VerticalFlow& buttons = column(8);
  buttons << agui::button("Nová hra", &this->window, [this] {
    this->open(Page::NewGame);
    this->newGame.open();  // after open(), which takes the keyboard from everything
  }, &theme.continueButton);
  buttons << agui::button("Načíst hru", &this->window, [this] {
    this->loadGame.open();
    this->open(Page::LoadGame);
  }, &theme.menuButton);
  buttons << agui::button("Nastavení", &this->window, [this] {
    this->settings.open();
    this->open(Page::Settings);
  }, &theme.menuButton);
  buttons << agui::button("O hře", &this->window, [this] { this->open(Page::About); }, &theme.menuButton);
  buttons << agui::button("Konec", &this->window, [this] { this->pending = Action::Quit; }, &theme.menuButton);
  this->window << buttons;

  // The game's menu: the secret password, and Start.
  const auto submitPassword = [this] { this->pending = Action::EnterPassword; };
  this->passwordField = &make<agui::TextField>();
  this->passwordField->style.setMinimalWidth(PASSWORD_W);
  this->passwordField->style.setMaximalWidth(PASSWORD_W);
  this->passwordField->onConfirm(this, submitPassword);
  agui::HorizontalFlow& secret = row(8);
  secret << agui::label("Tajné heslo:") << *this->passwordField << agui::button("OK", &this->game, submitPassword);
  agui::VerticalFlow& gameButtons = column(8);
  gameButtons << secret;
  gameButtons << agui::button("Start", &this->game, [this] {
    this->pending = Action::StartGoban;
    this->open(Page::Goban);
  }, &theme.continueButton);
  this->game << gameButtons;

  agui::VerticalFlow& gamePauseButtons = column(8);
  gamePauseButtons << agui::button("Uložit a odejít", &this->gamePause, [this] {
    this->pending = Action::SaveAndQuit;
    this->open(Page::Menu);
  }, &theme.menuButton);
  this->gamePause << gamePauseButtons;

  agui::VerticalFlow& gobanPauseButtons = column(8);
  gobanPauseButtons << agui::button("Zpět do menu", &this->gobanPause, [this] {
    this->pending = Action::LeaveGoban;
    this->open(Page::Game);
  }, &theme.menuButton);
  this->gobanPause << gobanPauseButtons;

  this->livesText    = &agui::label("", &theme.headingLabel);
  this->earningsText = &agui::label("");
  this->round << *this->livesText << *this->earningsText;

  // As wide as the properties under it.
  agui::Frame& moneyPanel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  moneyPanel.style.setMinimalWidth(PropertiesWindow::PANEL_W);
  this->moneyText = &agui::label("", &theme.headingLabel);
  moneyPanel << *this->moneyText;
  this->money << moneyPanel;

  for (size_t i = 0; i < std::size(UPGRADES); ++i) {
    this->shop.push_back(std::make_unique<UpgradeWindow>(theme, UPGRADES[i], [this, i] {
      this->bought  = i;
      this->pending = Action::BuyUpgrade;
    }));
  }

  this->dimmer.style.setParent(&theme.dimmer);

  gui.add(&this->window);
  gui.add(&this->newGame.root());
  gui.add(&this->loadGame.root());
  gui.add(&this->deleteGame.root());
  gui.add(&this->settings.root());
  gui.add(&this->about.root());
  gui.add(&this->game);
  gui.add(&this->figure);
  gui.add(&this->zombie);
  gui.add(&this->money);
  for (const auto& upgrade : this->shop) gui.add(upgrade.get());
  gui.add(&this->round);
  gui.add(&this->dimmer);  // before the pause menus, so they aren't dimmed too
  gui.add(&this->gamePause);
  gui.add(&this->gobanPause);
  this->open(Page::Menu);
}

MainMenu::~MainMenu()
{
  this->gui.remove(&this->gobanPause);
  this->gui.remove(&this->gamePause);
  this->gui.remove(&this->dimmer);
  this->gui.remove(&this->round);
  for (const auto& upgrade : this->shop) this->gui.remove(upgrade.get());
  this->gui.remove(&this->money);
  this->gui.remove(&this->zombie);
  this->gui.remove(&this->figure);
  this->gui.remove(&this->game);
  this->gui.remove(&this->about.root());
  this->gui.remove(&this->settings.root());
  this->gui.remove(&this->deleteGame.root());
  this->gui.remove(&this->loadGame.root());
  this->gui.remove(&this->newGame.root());
  this->gui.remove(&this->window);
}

void MainMenu::play(const ::Game& played)
{
  for (agui::Window* titled : { &this->game, &this->gamePause, &this->gobanPause }) titled->title.setText(std::string(played.name));
  this->clearPassword();
  this->showGame(played);
  this->open(Page::Game);
}

void MainMenu::showGame(const ::Game& shown)
{
  this->figure.show(shown.figure);
  this->zombie.show(shown.zombie);
  this->moneyText->setText(std::to_string(shown.money) + " Kč");
  for (size_t i = 0; i < this->shop.size(); ++i) this->shop[i]->show(shown.price(i), shown.money);
}

std::string MainMenu::password() const
{
  return this->passwordField->getText();
}

void MainMenu::clearPassword()
{
  this->passwordField->setText(std::string());
}

void MainMenu::showRound(int lives, int earnings)
{
  if (lives != this->livesShown) {
    this->livesShown = lives;
    this->livesText->setText(std::to_string(lives) + " HP");
  }
  if (earnings != this->earningsShown) {
    this->earningsShown = earnings;
    this->earningsText->setText("Výdělek: " + std::to_string(earnings) + " Kč");
  }
}


void MainMenu::open(Page p)
{
  this->page     = p;
  this->recentre = true;
  this->window.setVisible(p == Page::Menu);
  this->newGame.root().setVisible(p == Page::NewGame);
  this->loadGame.root().setVisible(p == Page::LoadGame);
  this->deleteGame.root().setVisible(p == Page::DeleteGame);
  this->settings.root().setVisible(p == Page::Settings);
  this->about.root().setVisible(p == Page::About);
  // The game's menu stays under its pause menu, darkened.
  this->game.setVisible(p == Page::Game || p == Page::GamePause);
  this->figure.setVisible(p == Page::Game || p == Page::GamePause);
  this->zombie.setVisible(p == Page::Game || p == Page::GamePause);
  this->money.setVisible(p == Page::Game || p == Page::GamePause);
  for (const auto& upgrade : this->shop) upgrade->setVisible(p == Page::Game || p == Page::GamePause);
  this->round.setVisible(p == Page::Goban || p == Page::GobanPause);
  this->dimmer.setVisible(p == Page::GamePause || p == Page::GobanPause);
  this->gamePause.setVisible(p == Page::GamePause);
  this->gobanPause.setVisible(p == Page::GobanPause);
  // Agui brings a window that is clicked to the front: after Start, the
  // game's menu would be over the sheet and its own pause menu, and hide it.
  if (p == Page::GamePause || p == Page::GobanPause) {
    this->dimmer.bringToFront();
    (p == Page::GamePause ? this->gamePause : this->gobanPause).bringToFront();
  }
  this->gui.clearFocus();
}

void MainMenu::cancel()
{
  if (this->page == Page::Settings && this->settings.searchBar().clearAndHide()) return;
  switch (this->page) {
  case Page::Game:       this->open(Page::GamePause); break;
  case Page::GamePause:  this->open(Page::Game); break;
  case Page::Goban:      this->open(Page::GobanPause); break;
  case Page::GobanPause: this->open(Page::Goban); break;
  case Page::Menu:    break;
  // Cancel: the save stays, and back to the list.
  case Page::DeleteGame: this->open(Page::LoadGame); break;
  // Back from Settings drops its draft: nothing to do but not keep it.
  default:            this->open(Page::Menu); break;
  }
}

void MainMenu::focusSearch()
{
  if (this->page == Page::Settings) this->settings.searchBar().focusSearch();
}

agui::Window* MainMenu::shown()
{
  switch (this->page) {
  case Page::Menu:     return &this->window;
  case Page::NewGame:  return &this->newGame.root();
  case Page::LoadGame: return &this->loadGame.root();
  case Page::DeleteGame: return &this->deleteGame.root();
  case Page::Settings: return &this->settings.root();
  case Page::About:    return &this->about.root();
  case Page::Game:     return &this->game;
  case Page::GamePause:  return &this->gamePause;
  case Page::GobanPause: return &this->gobanPause;
  case Page::Goban:    break;
  }
  return nullptr;
}

void MainMenu::layout(int screenWidth, int screenHeight)
{
  if (screenWidth != this->lastScreenWidth || screenHeight != this->lastScreenHeight) {
    this->lastScreenWidth  = screenWidth;
    this->lastScreenHeight = screenHeight;
    this->recentre = true;
  }

  this->dimmer.setLocation(0, 0);
  this->dimmer.setSize(screenWidth, screenHeight, agui::SetSizeInfo());

  // Up from the bottom right corner, a little way in: the figure's
  // properties, the zombie's over them, and the money over those.
  if (this->figure.isVisible()) {
    int top = screenHeight;
    for (agui::Window* stacked : { static_cast<agui::Window*>(&this->figure), static_cast<agui::Window*>(&this->zombie),
                                  &this->money }) {
      top -= stacked->getHeight() + CORNER_GAP;
      stacked->setLocation(screenWidth - stacked->getWidth() - CORNER_GAP, top);
    }
  }

  // The shop from the top left corner along the top, a little way in.
  int shopLeft = CORNER_GAP;
  for (const auto& upgrade : this->shop) {
    upgrade->setLocation(shopLeft, CORNER_GAP);
    shopLeft += upgrade->getWidth() + CORNER_GAP;
  }

  // The lives and earnings in the gap right of the goban: across, half way
  // between it and the edge of the screen, and level with its top. The goban
  // is laid out on the screen in pixels, this in GUI units, but it is in
  // proportion to the screen, so it lands in the same place near enough.
  // Where the gap is too narrow, in the corner.
  if (this->round.isVisible()) {
    const BoardLayout goban = LayOutBoard(screenWidth, screenHeight);
    const int gapLeft = goban.left + goban.side;
    const int gap     = screenWidth - gapLeft;
    if (gap >= this->round.getWidth() + 2 * CORNER_GAP) {
      this->round.setLocation(gapLeft + (gap - this->round.getWidth()) / 2, goban.top);
    } else {
      this->round.setLocation(screenWidth - this->round.getWidth() - CORNER_GAP, CORNER_GAP);
    }
  }

  // The game's menu under its pause menu stays in the middle too.
  if (this->page == Page::GamePause && this->game.getWidth() > 0) {
    this->game.setLocation((screenWidth - this->game.getWidth()) / 2, (screenHeight - this->game.getHeight()) / 2);
  }

  // Centred when it opens or the screen changes size; in between a page can
  // be dragged by its title bar, but is kept on screen.
  agui::Window* shown = this->shown();
  if (!shown) return;
  if (this->recentre && shown->getWidth() > 0) {  // not before its first layout
    shown->setLocation((screenWidth - shown->getWidth()) / 2, (screenHeight - shown->getHeight()) / 2);
    this->recentre = false;
  }
  shown->ensureWholeWindowIsOnScreen();
}

MainMenu::Action MainMenu::takeAction()
{
  const Action a = this->pending;
  this->pending = Action::None;
  return a;
}

}  // namespace ui

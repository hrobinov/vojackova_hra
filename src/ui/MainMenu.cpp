#include <ui/MainMenu.hpp>

#include <game/BoardView.hpp>

#include <ui/Form.hpp>
#include <ui/SearchBar.hpp>
#include <ui/Theme.hpp>

#include <Agui/Gui.hpp>
#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/TextField.hpp>

#include <algorithm>

namespace ui {

namespace {

// How far the game menu's windows and the round's lives and earnings are
// from the corner, and the windows apart.
constexpr int CORNER_GAP = 16;
// How wide the command line F3 opens is.
constexpr int COMMAND_W = 420;
// How big Start is; each checkpoint beside it is as tall and half as wide.
constexpr int START_W = 160;
constexpr int START_H = 50;  // menu_button's height

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
    , figure(theme, "Základní vlastnosti panáčka", true)
    , zombie(theme, "Základní vlastnosti bílého zombíka", false)
    , blackZombie(theme, "Základní vlastnosti černého zombíka", false)
    , redZombie(theme, "Základní vlastnosti červeného zombíka", false)
    , round(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding)
    , money(agui::GuiDirection::Vertical, "Peníze")
    , waveTitle(std::string(), &theme.headingLabel)
    , specialBox(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding)
    , specialTitle(std::string("Speciální schopnosti"), &theme.headingLabel)
    , commandLine(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding)
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

  // The game's menu: Start, with the checkpoints reached beside it.
  agui::VerticalFlow& gameButtons = column(8);
  const auto startAt = [this](int wave) {
    this->startingWave = wave;
    this->pending      = Action::StartGoban;
    this->open(Page::Goban);
  };
  agui::HorizontalFlow& starts = row(8);
  starts << footerButton("Start", &this->game, [startAt] { startAt(1); }, &theme.continueButton, START_W);
  for (int wave : ::Game::CHECKPOINTS) {
    const std::string text = "Kolo " + std::to_string(wave);
    // As tall as Start and half as wide: a plain button rather than a big one.
    agui::Button& checkpoint = footerButton(text.c_str(), &this->game, [startAt, wave] { startAt(wave); }, nullptr, START_W / 2);
    checkpoint.style.setMinimalHeight(START_H);
    checkpoint.style.setMaximalHeight(START_H);
    this->checkpoints.push_back(&checkpoint);
    starts << checkpoint;
  }
  gameButtons << starts;
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
  this->actionsText  = &agui::label("");
  this->earningsText = &agui::label("");
  this->round << *this->livesText << *this->actionsText << *this->earningsText;

  this->specials[0] = { Special::Rocket, "Rakety", "Aktivovat raketu", "Klikni na cíl" };
  this->specials[1] = { Special::Shotgun, "Brokovnice", "Aktivovat brokovnici", "Klikni vedle sebe" };
  agui::VerticalFlow& specialRows = column(12);
  for (SpecialRow& special : this->specials) {
    special.text   = &agui::label("", &theme.headingLabel);
    special.button = &agui::button(std::string(special.ready), &this->specialBox, [this, &special] {
      // Pressed again, put back; otherwise readied, and any other put back.
      const bool again = this->armed == special.special;
      this->disarm();
      if (!again) {
        this->armed = special.special;
        special.button->setText(std::string(special.aim));
      }
    });
    special.button->style.setHorizontallyStretchable(true);
    special.row = &column(4);
    *special.row << *special.text << *special.button;
    specialRows << *special.row;
  }
  this->specialBox << specialRows;

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

  this->commandField = &make<agui::TextField>();
  this->commandField->style.setMinimalWidth(COMMAND_W);
  this->commandField->style.setMaximalWidth(COMMAND_W);
  this->commandField->onConfirm(this, [this] {
    this->sent    = this->commandField->getText();
    this->pending = Action::Command;
    this->toggleCommandLine();
  });
  this->commandLine << *this->commandField;
  this->commandLine.setVisible(false);

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
  gui.add(&this->blackZombie);
  gui.add(&this->redZombie);
  gui.add(&this->money);
  for (const auto& upgrade : this->shop) gui.add(upgrade.get());
  gui.add(&this->specialTitle);
  gui.add(&this->round);
  gui.add(&this->waveTitle);
  gui.add(&this->specialBox);
  gui.add(&this->dimmer);  // before the pause menus, so they aren't dimmed too
  gui.add(&this->gamePause);
  gui.add(&this->gobanPause);
  gui.add(&this->commandLine);
  this->open(Page::Menu);
}

MainMenu::~MainMenu()
{
  this->gui.remove(&this->commandLine);
  this->gui.remove(&this->gobanPause);
  this->gui.remove(&this->gamePause);
  this->gui.remove(&this->dimmer);
  this->gui.remove(&this->specialBox);
  this->gui.remove(&this->waveTitle);
  this->gui.remove(&this->round);
  this->gui.remove(&this->specialTitle);
  for (const auto& upgrade : this->shop) this->gui.remove(upgrade.get());
  this->gui.remove(&this->money);
  this->gui.remove(&this->redZombie);
  this->gui.remove(&this->blackZombie);
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
  this->showGame(played);
  this->open(Page::Game);
}

void MainMenu::showGame(const ::Game& shown)
{
  this->figure.show(shown.figure);
  this->zombie.show(shown.zombie);
  this->blackZombie.show(shown.blackZombie);
  this->redZombie.show(shown.redZombie);
  this->moneyText->setText(std::to_string(shown.money) + " Kč");
  for (size_t i = 0; i < this->shop.size(); ++i) this->shop[i]->show(shown.price(i), shown.money);
  for (size_t i = 0; i < this->checkpoints.size(); ++i) {
    this->checkpoints[i]->setVisible(shown.furthestWave >= ::Game::CHECKPOINTS[i]);
  }
}

void MainMenu::toggleCommandLine()
{
  const bool opening = !this->commandLine.isVisible();
  this->commandField->setText(std::string());
  this->commandLine.setVisible(opening);
  if (opening) {
    this->commandLine.bringToFront();
    this->commandField->focus();
  } else {
    this->gui.clearFocus();
  }
}

bool MainMenu::commandLineOpen() const
{
  return this->commandLine.isVisible();
}

void MainMenu::disarm()
{
  this->armed = Special::None;
  for (SpecialRow& special : this->specials) special.button->setText(std::string(special.ready));
}

void MainMenu::showRound(const Goban& goban)
{
  const int wave     = goban.waveNumber();
  const int lives    = goban.figureLives();
  const int earnings = goban.earnings();
  if (goban.figureActions() != this->actionsShown) {
    this->actionsShown = goban.figureActions();
    this->actionsText->setText("Akce: " + std::to_string(this->actionsShown));
  }
  // A special ability only once the figure has one at all, the box only with
  // any; a button only with some left this round.
  bool any = false;
  for (SpecialRow& special : this->specials) {
    const bool rocket = special.special == Special::Rocket;
    const int  left   = rocket ? goban.rocketsLeft() : goban.shellsLeft();
    const int  owned  = rocket ? goban.figureStart().rockets : goban.figureStart().shells;
    special.row->setVisible(owned > 0);
    any = any || owned > 0;
    if (left == special.shown) continue;
    special.shown = left;
    special.text->setText(std::string(special.name) + ": " + std::to_string(left));
    special.button->setEnabled(left > 0);
    if (left == 0 && this->armed == special.special) this->disarm();
  }
  this->specialBox.setVisible(any && this->gobanShown());
  if (wave != this->waveShown) {
    this->waveShown = wave;
    this->waveTitle.setText("Kolo: " + std::to_string(wave));
  }
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
  // Back in the main menu, no game is open to type to.
  if (p == Page::Menu && this->commandLineOpen()) this->toggleCommandLine();
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
  this->blackZombie.setVisible(p == Page::Game || p == Page::GamePause);
  this->redZombie.setVisible(p == Page::Game || p == Page::GamePause);
  this->money.setVisible(p == Page::Game || p == Page::GamePause);
  for (const auto& upgrade : this->shop) upgrade->setVisible(p == Page::Game || p == Page::GamePause);
  this->specialTitle.setVisible(p == Page::Game || p == Page::GamePause);
  this->round.setVisible(p == Page::Goban || p == Page::GobanPause);
  this->waveTitle.setVisible(p == Page::Goban || p == Page::GobanPause);
  if (p != Page::Goban && p != Page::GobanPause) {
    this->specialBox.setVisible(false);
    this->disarm();
  }
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
  if (this->commandLineOpen()) {
    this->toggleCommandLine();
    return;
  }
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
  // properties, the white zombie's over them, the black one's and the red
  // one's over those, and the money on top.
  if (this->figure.isVisible()) {
    int top = screenHeight;
    for (agui::Window* stacked : { static_cast<agui::Window*>(&this->figure), static_cast<agui::Window*>(&this->zombie),
                                  static_cast<agui::Window*>(&this->blackZombie), static_cast<agui::Window*>(&this->redZombie),
                                  &this->money }) {
      top -= stacked->getHeight() + CORNER_GAP;
      stacked->setLocation(screenWidth - stacked->getWidth() - CORNER_GAP, top);
    }
  }

  // The command line in the bottom left corner.
  this->commandLine.setLocation(CORNER_GAP, screenHeight - this->commandLine.getHeight() - CORNER_GAP);

  // The shop from the top left corner, a little way in: the upgrades along
  // the top, and under them down the left, headed, the special abilities.
  int shopLeft = CORNER_GAP, shopBottom = CORNER_GAP;
  for (size_t i = 0; i < this->shop.size(); ++i) {
    if (UPGRADES[i].special) continue;
    this->shop[i]->setLocation(shopLeft, CORNER_GAP);
    shopLeft += this->shop[i]->getWidth() + CORNER_GAP;
    shopBottom = std::max(shopBottom, CORNER_GAP + this->shop[i]->getHeight());
  }
  this->specialTitle.setLocation(CORNER_GAP, shopBottom + 2 * CORNER_GAP);
  int specialTop = shopBottom + 2 * CORNER_GAP + this->specialTitle.getHeight() + CORNER_GAP / 2;
  for (size_t i = 0; i < this->shop.size(); ++i) {
    if (!UPGRADES[i].special) continue;
    this->shop[i]->setLocation(CORNER_GAP, specialTop);
    specialTop += this->shop[i]->getHeight() + CORNER_GAP;
  }

  // The lives and earnings in the gap right of the goban: across, half way
  // between it and the edge of the screen, and level with its top. The goban
  // is laid out on the screen in pixels, this in GUI units, but it is in
  // proportion to the screen, so it lands in the same place near enough.
  // Where the gap is too narrow, in the corner.
  if (this->round.isVisible()) {
    const BoardLayout goban = LayOutBoard(screenWidth, screenHeight);
    // The special abilities the same way in the gap left of it.
    if (goban.left >= this->specialBox.getWidth() + 2 * CORNER_GAP) {
      this->specialBox.setLocation((goban.left - this->specialBox.getWidth()) / 2, goban.top);
    } else {
      this->specialBox.setLocation(CORNER_GAP, CORNER_GAP);
    }
    // The wave in the middle of the gap over the goban.
    this->waveTitle.setLocation((screenWidth - this->waveTitle.getWidth()) / 2,
                                std::max(0, (goban.top - this->waveTitle.getHeight()) / 2));
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

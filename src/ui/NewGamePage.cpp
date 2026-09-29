#include <ui/NewGamePage.hpp>

#include <game/Game.hpp>
#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/KeyEvent.hpp>
#include <Agui/Widget/TextField.hpp>

#include <cstdint>
#include <string_view>

namespace ui {

namespace {

constexpr int FIELD_W = 220;

// UTF-8 continuation bytes are 10xxxxxx; every other byte starts a character.
bool StartsCharacter(char c)
{
  return (static_cast<unsigned char>(c) & 0xC0) != 0x80;
}

size_t Characters(std::string_view text)
{
  size_t count = 0;
  for (char c : text) count += StartsCharacter(c);
  return count;
}

// A text field that takes no more than Game::MAX_NAME characters: a typed or
// pasted character that wouldn't fit is refused before it is written, and
// `onRefused` told. (Agui's own limit counts bytes, and so would give a name
// with č or ř fewer letters. And cutting the text back once it is written
// isn't safe: Agui then puts the caret after the character it just added,
// past the end.)
class NameField : public agui::TextField {
public:
  explicit NameField(std::function<void()> onRefused)
      : onRefused(std::move(onRefused))
  {}

protected:
  void handleKeyboard(const agui::KeyEvent& keyEvent) override
  {
    const uint32_t c = keyEvent.getUnichar();
    if (c >= ' ' && c != 0x7F && !this->fits(1)) {
      this->onRefused();
      return;
    }
    agui::TextField::handleKeyboard(keyEvent);
  }

  bool canPasteText(std::string_view pasted) const override
  {
    if (this->fits(Characters(pasted))) return agui::TextField::canPasteText(pasted);
    this->onRefused();
    return false;
  }

private:
  // Whether `added` more characters fit, in place of whatever is selected.
  bool fits(size_t added) const
  {
    const size_t selected = size_t(this->getSelectionEnd() - this->getSelectionStart());
    return Characters(this->getText()) - selected + added <= Game::MAX_NAME;
  }

  std::function<void()> onRefused;
};

std::string Trimmed(const std::string& text)
{
  const size_t first = text.find_first_not_of(' ');
  if (first == std::string::npos) return {};
  return text.substr(first, text.find_last_not_of(' ') - first + 1);
}

}  // namespace

NewGamePage::NewGamePage(Theme& theme, std::function<void()> onDone, std::function<void()> onBack)
    : window(agui::GuiDirection::Vertical, "Nová hra")
    , onDone(std::move(onDone))
{
  this->window.setDragTarget(&this->window);

  this->field = &make<NameField>([this] { this->tooLong->setVisible(true); });
  this->field->style.setMinimalWidth(FIELD_W);
  this->field->style.setMaximalWidth(FIELD_W);
  this->field->onTextEdit(this, [this] { this->edited(); });
  this->field->onConfirm(this, [this] { this->done(); });
  this->counter = &agui::label("", &theme.dimLabel);

  agui::HorizontalFlow& line = namedRow("Jméno hry", *this->field, 100);
  line << *this->counter;
  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  this->tooLong = &agui::label("Jméno hry může mít nejvýš " + std::to_string(Game::MAX_NAME) + " znaků.", &theme.badLabel);
  panel << line << *this->tooLong;
  this->window << panel;

  agui::HorizontalFlow& footer = row(8);
  footer.style.setTopPadding(8);
  footer.style.setHorizontallyStretchable(true);
  footer << agui::button("Zpět", &this->window, std::move(onBack), &theme.backButton);
  footer << dragHandle(&theme.draggableSpace, &this->window);
  this->finish = &footerButton("Hotovo", &this->window, [this] { this->done(); }, &theme.forwardButton, 160);
  footer << *this->finish;
  this->window << footer;

  this->edited();
}

void NewGamePage::open()
{
  this->field->setText(std::string());
  this->edited();
  this->field->focus();
}

std::string NewGamePage::name() const
{
  return Trimmed(this->field->getText());
}

void NewGamePage::edited()
{
  // The red line stays until the next edit that fits.
  this->tooLong->setVisible(false);
  this->counter->setText(std::to_string(Characters(this->field->getText())) + " / " + std::to_string(Game::MAX_NAME));
  this->finish->setEnabled(!this->name().empty());
}

void NewGamePage::done()
{
  if (!this->name().empty()) this->onDone();
}

}  // namespace ui

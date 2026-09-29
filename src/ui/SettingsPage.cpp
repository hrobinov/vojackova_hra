#include <ui/SettingsPage.hpp>

#include <ui/Form.hpp>
#include <ui/SearchBar.hpp>
#include <ui/Theme.hpp>

#include <Agui/LowercaseString.hpp>
#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/CheckBox.hpp>
#include <Agui/Widget/DropDown.hpp>
#include <Agui/Widget/Frame.hpp>
#include <Agui/Widget/HorizontalFlow.hpp>
#include <Agui/Widget/ImageWidget.hpp>
#include <Agui/Widget/Label.hpp>
#include <Agui/Widget/RadioButton.hpp>
#include <Agui/Widget/Slider.hpp>
#include <Agui/Widget/TextField.hpp>
#include <Agui/Widget/VerticalFlow.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>

namespace ui {

namespace {

// What the reset button puts back: the settings a first run starts with.
const Settings DEFAULTS{};

// The info icon after a name that has a tooltip: as tall as a line of text.
constexpr int INFO_W = 8;
constexpr int INFO_H = 20;

// other_settings_slider.
constexpr int SLIDER_PX = 250;

std::string Percent(int value)
{
  return std::to_string(value) + " %";
}

std::string DelayText(int delay)
{
  if (delay == Settings::Graphics::TOOLTIPS_NEVER) return "Nikdy";
  if (delay == 0) return "Okamžitě";
  return std::to_string(delay) + " ms";
}

}  // namespace

SettingsPage::SettingsPage(Theme& theme, const Settings& live, std::function<void()> onConfirm, std::function<void()> onBack)
    : live(live)
    , settings(live)
    , openedWith(live)
    , theme(theme)
    , window(agui::GuiDirection::Vertical, "Nastavení")
    , resettable(theme,
                 [this] {
                   this->settings.graphics = DEFAULTS.graphics;
                   this->refresh();
                 })
{
  this->window.setDragTarget(&this->window);
  using Graphics = Settings::Graphics;

  // scroll_pane_under_subheader's padding round the bordered frames.
  agui::VerticalFlow& content = column(4);
  content.style.setPaddings(4, 4, 4, 4);
  content.style.setMinimalWidth(480);

  this->mode = &make<agui::DropDown>();
  this->mode->addItems({ "V okně", "V okně (celá obrazovka)" });
  this->mode->onItemSelect(this, [this](int i) {
    this->settings.graphics.windowedFullscreen = i == 1;
    this->changed();
  });
  this->settingRow(this->section(content), "Režim zobrazení",
                   "V okně (celá obrazovka) je okno bez okrajů přes celý monitor. Monitor si nechá svůj režim, "
                   "a Alt+Tab i ostatní okna fungují jako obvykle.",
                   *this->mode);
  this->resettable.track(*this->mode, [this](const Settings& other) {
    return this->settings.graphics.windowedFullscreen != other.graphics.windowedFullscreen;
  });

  // As Factorio's UI scale: automatic, following the window, or a custom
  // one on a notched slider -- which, with its value, is only live while
  // custom is picked.
  {
    agui::Frame& scale = this->section(content);
    // Found as a whole, by its caption or its choices, not choice by choice.
    scale.setAtomicSearch();
    const std::string tip = "Jak velké je celé rozhraní. Automatická velikost se řídí velikostí okna. Kdekoli ve hře ji "
                            "mění " + ShortcutText("Ctrl + Num +") + " a " + ShortcutText("Ctrl + Num -") + ". " +
                            ShortcutText("Ctrl + Num 0") + " vrátí automatickou.";
    scale << this->name("Velikost rozhraní", tip.c_str(), &this->theme.captionLabel);

    this->automaticScale = &make<agui::RadioButton>(std::string("Automatická"));
    this->manualScale    = &make<agui::RadioButton>(std::string("Vlastní"));
    this->scaleChoice.add(this->automaticScale);
    this->scaleChoice.add(this->manualScale);
    this->automaticScale->onCheckChange(this, [this](bool on) {
      if (!on) return;
      this->settings.graphics.automaticScale = true;
      this->refresh();
    });
    this->manualScale->onCheckChange(this, [this](bool on) {
      if (!on) return;
      this->settings.graphics.automaticScale = false;
      this->refresh();
    });
    scale << *this->automaticScale;

    this->manualScaleSlider = &make<agui::Slider>(&this->theme.notchedSlider);
    this->manualScaleSlider->setMinMaxValues(Graphics::MIN_SCALE, Graphics::MAX_SCALE);
    this->manualScaleSlider->setValueStep(Graphics::SCALE_STEP);
    this->manualScaleSlider->setDiscreteSlider();
    this->manualScaleSlider->style.setMinimalWidth(SLIDER_PX);
    this->manualScaleSlider->onSliderMove(this, [this](double v) {
      this->settings.graphics.interfaceScale = ClampedInterfaceScale(int(std::lround(v)));
      this->refresh();
    });
    this->manualScaleValue = &make<agui::TextField>(&this->theme.sliderValueField);
    this->manualScaleValue->onConfirm(this, [this] { this->typedManualScale(); });
    this->manualScaleValue->onFocusLose(this, [this] { this->typedManualScale(); });
    agui::HorizontalFlow& manual = make<agui::HorizontalFlow>(&this->theme.playerInputFlow);
    manual << *this->manualScale << agui::pusher
           << *this->manualScaleSlider << *this->manualScaleValue;
    scale << manual;

    this->resettable.track(*this->automaticScale, [this](const Settings& other) {
      return this->settings.graphics.automaticScale != other.graphics.automaticScale;
    });
    this->resettable.track(this->manualScaleSlider->marker, [this](const Settings& other) {
      return this->settings.graphics.interfaceScale != other.graphics.interfaceScale;
    });
  }

  this->fpsLimits = { 0, 30, 60, 120, 144, 240 };
  // A hand-edited limit that isn't one of the choices gets an item of its own.
  if (std::find(this->fpsLimits.begin(), this->fpsLimits.end(), live.graphics.fpsLimit) == this->fpsLimits.end()) {
    this->fpsLimits.push_back(live.graphics.fpsLimit);
  }
  this->fps = &make<agui::DropDown>();
  for (int limit : this->fpsLimits) this->fps->addItem(limit ? std::to_string(limit) : std::string("Žádné"));
  this->fps->onItemSelect(this, [this](int i) {
    this->settings.graphics.fpsLimit = this->fpsLimits[size_t(i)];
    this->changed();
  });
  this->settingRow(this->section(content), "Omezení snímků za sekundu", nullptr, *this->fps);
  this->resettable.track(*this->fps, [this](const Settings& other) { return this->settings.graphics.fpsLimit != other.graphics.fpsLimit; });

  // Like the autosave interval: a caption, then the slider and a text field
  // that shows its value and takes a typed one.
  {
    agui::Frame& delay = this->section(content);
    delay.setAtomicSearch();
    const std::string tip = "Jak dlouho musí myš na něčem stát, než se ukáže nápověda. Se stisknutým " +
                            ShortcutText("Shift") + " se nápovědy ukážou hned, ať je tu nastaveno cokoli.";
    delay << this->name("Zpoždění nápovědy", tip.c_str(), &this->theme.captionLabel);
    // 0 to 200 ms, and one notch past the end for "never".
    this->tooltipDelay = &make<agui::Slider>();
    this->tooltipDelay->setMinMaxValues(0, Graphics::MAX_TOOLTIP_DELAY + Graphics::TOOLTIP_DELAY_STEP);
    this->tooltipDelay->setValueStep(Graphics::TOOLTIP_DELAY_STEP);
    this->tooltipDelay->style.setMinimalWidth(SLIDER_PX);
    this->tooltipDelay->onSliderMove(this, [this](double v) {
      const int ms = int(std::lround(v));
      this->settings.graphics.tooltipDelay = ms > Graphics::MAX_TOOLTIP_DELAY ? Graphics::TOOLTIPS_NEVER : ms;
      this->refresh();
    });
    this->tooltipDelayValue = &make<agui::TextField>(&this->theme.sliderValueField);
    this->tooltipDelayValue->onConfirm(this, [this] { this->typedTooltipDelay(); });
    this->tooltipDelayValue->onFocusLose(this, [this] { this->typedTooltipDelay(); });
    agui::HorizontalFlow& row = make<agui::HorizontalFlow>(&this->theme.playerInputFlow);
    row << *this->tooltipDelay << agui::pusher << *this->tooltipDelayValue;
    delay << row;
    // Factorio lights up a slider's knob.
    this->resettable.track(this->tooltipDelay->marker, [this](const Settings& other) {
      return this->settings.graphics.tooltipDelay != other.graphics.tooltipDelay;
    });
  }

  this->vsync = &make<agui::CheckBox>(std::string("VSync"));
  this->vsync->onCheckChange(this, [this](bool on) {
    this->settings.graphics.vsync = on;
    this->changed();
  });
  this->checkRow(this->section(content), *this->vsync,
                 "Mezi snímky počkat na monitor: obraz se netrhá a snímků není víc, než kolik jich monitor ukáže.");
  this->resettable.track(*this->vsync, [this](const Settings& other) { return this->settings.graphics.vsync != other.graphics.vsync; });

  agui::HorizontalFlow& strip = row();
  strip.style.setHorizontallyStretchable(true);
  // The subheader, and the reset button at its right end.
  strip << agui::pusher << this->resettable.resetButton();
  agui::Frame& subheader = make<agui::Frame>(agui::GuiDirection::Horizontal, &theme.subheaderFrame);
  subheader << strip;

  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrame);
  panel << subheader << content;
  this->window << panel;

  // Search, in the title bar: what doesn't match is hidden, section by
  // section and row by row, the way Factorio's settings windows search.
  content.neverHideBySearch();
  this->search = &make<SearchBar>(theme, this->window, [flow = &content](const std::string& text) {
    flow->genericSearch(agui::LowercaseString(text));
  });
  this->search->keepSizeOf(panel);
  this->search->setShortcut("Ctrl + F");

  // dialog_buttons_horizontal_flow: Back throws the changes away, and while
  // the mouse is on it lights up what it would throw away; Confirm keeps them.
  agui::Button& back = agui::button("Zpět", &this->window, std::move(onBack), &theme.backButton);
  this->resettable.lightWhileHovered(back, [this]() -> const Settings& { return this->openedWith; });
  agui::HorizontalFlow& footer = row(8);
  footer.style.setTopPadding(8);
  footer.style.setHorizontallyStretchable(true);
  footer << back;
  footer << dragHandle(&theme.draggableSpace, &this->window);
  footer << footerButton("Potvrdit", &this->window, std::move(onConfirm), &theme.forwardButton, 200);
  this->window << footer;

  this->refresh();
}

agui::Frame& SettingsPage::section(agui::VerticalFlow& content)
{
  agui::Frame& frame = make<agui::Frame>(agui::GuiDirection::Vertical, &this->theme.borderedFrame);
  content << frame;
  return frame;
}

agui::Widget& SettingsPage::name(const char* text, const char* tip, const agui::LabelStyle* style)
{
  agui::Label& label = style ? agui::label(text, style) : agui::label(text);
  // A caption that names a setting is what search finds it by.
  if (style == &this->theme.captionLabel) label.dontIgnoreBySearch();
  if (!tip) return label;
  // Factorio's setToolTipWithInfoIcon puts an info icon after the text, and
  // either of them shows the tooltip.
  label.setToolTip(tip);
  agui::HorizontalFlow& flow = row(4);
  flow << label << this->info(tip);
  return flow;
}

agui::ImageWidget& SettingsPage::info(const char* tip)
{
  agui::ImageWidget& icon = make<agui::ImageWidget>(this->theme.infoIcon());
  icon.style.setMinimalWidth(INFO_W);
  icon.style.setMaximalWidth(INFO_W);
  icon.style.setMinimalHeight(INFO_H);
  icon.style.setMaximalHeight(INFO_H);
  icon.setToolTip(tip);
  return icon;
}

void SettingsPage::settingRow(agui::Frame& section, const char* name, const char* tip, agui::Widget& control)
{
  agui::HorizontalFlow& flow = make<agui::HorizontalFlow>(&this->theme.playerInputFlow);
  flow << this->name(name, tip) << agui::pusher << control;
  section << flow;
}

void SettingsPage::checkRow(agui::Frame& section, agui::CheckBox& box, const char* tip)
{
  if (!tip) {
    section << box;
    return;
  }
  box.setToolTip(tip);
  agui::HorizontalFlow& flow = row(4);
  flow.style.setVerticalAlign(agui::VerticalAlign::Center);
  flow << box << this->info(tip);
  section << flow;
}

void SettingsPage::setAutomaticScale(int percent)
{
  if (percent == this->automatic) return;
  this->automatic = percent;
  this->automaticScale->setText("Automatická (" + Percent(percent) + ")");
}

void SettingsPage::typedManualScale()
{
  const std::string text = this->manualScaleValue->getText();
  if (const size_t digit = text.find_first_of("0123456789"); digit != std::string::npos) {
    this->settings.graphics.interfaceScale = ClampedInterfaceScale(std::stoi(text.substr(digit, 9)));
  }
  // Anything else puts back what was there.
  this->manualScaleValue->setText(Percent(this->settings.graphics.interfaceScale));
  this->refresh();
}

void SettingsPage::open()
{
  this->search->clearAndHide();
  this->settings   = this->live;
  this->openedWith = this->live;
  this->refresh();
}

void SettingsPage::refresh()
{
  using Graphics = Settings::Graphics;
  const Graphics& g = this->settings.graphics;
  this->mode->setSelectedIndex(g.windowedFullscreen ? 1 : 0);
  if (this->vsync->isChecked() != g.vsync) this->vsync->setChecked(g.vsync);
  const auto limit = std::find(this->fpsLimits.begin(), this->fpsLimits.end(), g.fpsLimit);
  if (limit != this->fpsLimits.end()) this->fps->setSelectedIndex(int(limit - this->fpsLimits.begin()));

  agui::RadioButton* chosen = g.automaticScale ? this->automaticScale : this->manualScale;
  if (!chosen->isChecked()) this->scaleChoice.setOnlySelected(chosen);
  this->manualScaleSlider->setValue(g.interfaceScale);
  if (!this->manualScaleValue->isFocused()) this->manualScaleValue->setText(Percent(g.interfaceScale));
  // Factorio's disabled look for the custom scale while it isn't the one used.
  if (this->manualScaleSlider->isEnabled() == g.automaticScale) {
    this->manualScaleSlider->setEnabled(!g.automaticScale);
    this->manualScaleValue->setEnabled(!g.automaticScale);
  }

  const int delay = g.tooltipDelay;
  this->tooltipDelay->setValue(delay == Graphics::TOOLTIPS_NEVER ? Graphics::MAX_TOOLTIP_DELAY + Graphics::TOOLTIP_DELAY_STEP : delay);
  // Not under the cursor of someone typing in it.
  if (!this->tooltipDelayValue->isFocused()) this->tooltipDelayValue->setText(DelayText(delay));

  this->changed();
}

void SettingsPage::typedTooltipDelay()
{
  using Graphics = Settings::Graphics;
  std::string text = this->tooltipDelayValue->getText();
  for (char& c : text) c = char(std::tolower(static_cast<unsigned char>(c)));

  int& delay = this->settings.graphics.tooltipDelay;
  if (text.find("nikdy") != std::string::npos) {
    delay = Graphics::TOOLTIPS_NEVER;
  } else if (text.find("okam") != std::string::npos) {  // okamžitě, whatever the ž came out as
    delay = 0;
  } else if (const size_t digit = text.find_first_of("0123456789"); digit != std::string::npos) {
    // The nearest step the slider has.
    const int ms = std::stoi(text.substr(digit, 9));
    const int step = Graphics::TOOLTIP_DELAY_STEP;
    delay = std::clamp((ms + step / 2) / step * step, 0, Graphics::MAX_TOOLTIP_DELAY);
  }
  // Anything else puts back what was there.
  this->tooltipDelayValue->setText(DelayText(delay));
  this->refresh();
}

void SettingsPage::changed()
{
  this->resettable.changed();
}

}  // namespace ui

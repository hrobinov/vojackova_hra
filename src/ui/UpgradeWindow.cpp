#include <ui/UpgradeWindow.hpp>

#include <ui/Form.hpp>
#include <ui/Theme.hpp>

#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/Frame.hpp>

#include <string>
#include <utility>

namespace ui {

namespace {

constexpr int BUTTON_W = 120;

}  // namespace

UpgradeWindow::UpgradeWindow(Theme& theme, const Upgrade& upgrade, std::function<void()> onBuy)
    : agui::Window(agui::GuiDirection::Vertical, upgrade.name)
{
  this->buy = &agui::button(std::string(), this, std::move(onBuy));
  this->buy->style.setMinimalWidth(BUTTON_W);
  agui::Frame& panel = make<agui::Frame>(agui::GuiDirection::Vertical, &theme.insideShallowFrameWithPadding);
  panel << *this->buy;
  *this << panel;
}

void UpgradeWindow::show(std::optional<int> price, int money)
{
  this->buy->setText(price ? std::to_string(*price) + " Kč" : std::string("Vyprodáno"));
  this->buy->setEnabled(price && money >= *price);
}

}  // namespace ui

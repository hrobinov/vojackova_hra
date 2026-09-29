// One upgrade in the shop of the game's menu: a small window named after it,
// and a button with its price now that buys it -- there to press only when
// the money is there, and once it can't be bought any more, sold out.

#pragma once

#include <game/Upgrade.hpp>

#include <Agui/Widget/Window.hpp>

#include <functional>
#include <optional>

namespace agui {
class Button;
}

namespace ui {

class Theme;

class UpgradeWindow : public agui::Window {
public:
  UpgradeWindow(Theme& theme, const Upgrade& upgrade, std::function<void()> onBuy);

  // What it costs now, or nothing when sold out, and the money there is.
  void show(std::optional<int> price, int money);

private:
  agui::Button* buy = nullptr;
};

}  // namespace ui

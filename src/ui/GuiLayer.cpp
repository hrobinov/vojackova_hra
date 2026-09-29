#include <ui/GuiLayer.hpp>

#include <app/Settings.hpp>
#include <ui/MainMenu.hpp>
#include <ui/Theme.hpp>

#include <Agui/Gui.hpp>
#include <Agui/TopContainer.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <rlgl.h>

namespace ui {

GuiLayer::GuiLayer(const Settings& settings)
{
  // The scale the GUI will be drawn at, which the Theme rasterises its fonts
  // for from the start rather than at 100% and then again.
  const int percent = EffectiveInterfaceScale(settings.graphics, GetScreenWidth(), GetScreenHeight());

  // Theme loads fonts through Agui, so the loader has to be in place first.
  agui::Font::setFontLoader(&this->fontLoader);
  this->theme = std::make_unique<Theme>(float(percent) / 100.0f);

  this->gui = std::make_unique<agui::Gui>();
  this->gui->setGraphics(&this->graphics);
  this->gui->setInput(&this->input);
  this->gui->setCursorProvider(&this->cursor);
  this->gui->resizeToDisplay();

  this->mainMenu = std::make_unique<MainMenu>(*this->gui, *this->theme, settings);

  this->setScale(percent);
  this->setTooltipDelay(settings.graphics.tooltipDelay);
}

GuiLayer::~GuiLayer()
{
  this->mainMenu.reset();
  this->gui.reset();
  this->theme.reset();
}

void GuiLayer::setScale(int percent)
{
  const float scale = float(percent) / 100.0f;
  this->graphics.setViewScale(scale);
  this->input.setViewScale(scale);
  this->theme->setScale(scale);
}

void GuiLayer::setTooltipDelay(int milliseconds)
{
  // Agui's own "never" is -1 seconds.
  this->gui->setGuiTooltipHoverInterval(milliseconds < 0 ? -1.0 : double(milliseconds) / 1000.0);
  this->gui->resetGuiTooltipHoverTime();
}

void GuiLayer::update()
{
  // Compared rather than asking IsWindowResized(): a resize made mid-frame, like
  // toggling fullscreen from the settings page, is cleared by the next
  // EndDrawing() before this ever sees it. In GUI units, so a change of scale
  // is a resize too. Not while minimized: Windows reports that as 0x0, and
  // laying out for it would recentre everything the player had dragged
  // somewhere.
  const agui::Dimension display = this->graphics.getDisplaySize();
  if (!IsWindowMinimized() && (display.width != this->screenWidth || display.height != this->screenHeight)) {
    this->screenWidth  = display.width;
    this->screenHeight = display.height;
    this->gui->resizeToDisplay();
  }
  // Shift shows tooltips at once, whatever the delay -- even when it is
  // "never". Agui takes back the ones it showed when Shift is let go of.
  this->gui->setInstantTooltip(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
  this->gui->logic(true);

  // A tooltip is a window like any other, and would take the mouse when it
  // moved onto one -- dropping the tooltip, which then comes back, over and
  // over. The mouse goes through them to what is under them.
  for (agui::Widget* child : this->gui->getTop()->getChildren()) {
    if (dynamic_cast<agui::ToolTip*>(child) && !child->isIgnoredByInteraction()) child->setIgnoredByInteraction(true);
  }

  this->mainMenu->layout(this->screenWidth, this->screenHeight);
}

void GuiLayer::draw()
{
  // Agui draws in GUI units; the matrix turns them into screen pixels.
  rlPushMatrix();
  rlScalef(this->graphics.getViewScale(), this->graphics.getViewScale(), 1.0f);
  this->graphics._beginPaint();
  this->gui->render();
  this->graphics._endPaint();
  rlPopMatrix();
}

}  // namespace ui

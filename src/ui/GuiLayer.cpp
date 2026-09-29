#include <ui/GuiLayer.hpp>

#include <ui/MainMenu.hpp>
#include <ui/Theme.hpp>

#include <Agui/Gui.hpp>
#include <rlgl.h>

#include <algorithm>

namespace ui {

namespace {

// The interface scale for a window `width` x `height` pixels, as Factorio's
// automatic UI scale works it out: in proportion to a 1920x1080 screen less
// the window's frame, by whichever way it is tighter, rounded down to a step
// of 25%. So 100% on full HD.
int AutomaticInterfaceScale(int width, int height)
{
  constexpr int STEP = 25, MIN = 75, MAX = 200;
  constexpr int FULL_HD_W = 1920 - 64, FULL_HD_H = 1080 - 64;
  const int fit = std::min(width * 100 / FULL_HD_W, height * 100 / FULL_HD_H);
  return std::clamp(fit / STEP * STEP, MIN, MAX);
}

}  // namespace

GuiLayer::GuiLayer()
{
  const int percent = AutomaticInterfaceScale(GetScreenWidth(), GetScreenHeight());

  // Theme loads fonts through Agui, so the loader has to be in place first.
  agui::Font::setFontLoader(&this->fontLoader);
  this->theme = std::make_unique<Theme>(float(percent) / 100.0f);

  this->gui = std::make_unique<agui::Gui>();
  this->gui->setGraphics(&this->graphics);
  this->gui->setInput(&this->input);
  this->gui->setCursorProvider(&this->cursor);
  this->gui->resizeToDisplay();

  this->mainMenu = std::make_unique<MainMenu>(*this->gui, *this->theme);

  this->setScale(percent);
}

GuiLayer::~GuiLayer()
{
  this->mainMenu.reset();
  this->gui.reset();
  this->theme.reset();
}

void GuiLayer::setScale(int percent)
{
  this->scale = percent;
  const float factor = float(percent) / 100.0f;
  this->graphics.setViewScale(factor);
  this->input.setViewScale(factor);
  this->theme->setScale(factor);
}

void GuiLayer::update()
{
  // Not while minimized: Windows reports that as 0x0.
  if (!IsWindowMinimized()) {
    // The scale follows the window as it is resized.
    if (const int percent = AutomaticInterfaceScale(GetScreenWidth(), GetScreenHeight()); percent != this->scale) {
      this->setScale(percent);
    }
    // In GUI units, so a change of scale is a resize too.
    const agui::Dimension display = this->graphics.getDisplaySize();
    if (display.width != this->screenWidth || display.height != this->screenHeight) {
      this->screenWidth  = display.width;
      this->screenHeight = display.height;
      this->gui->resizeToDisplay();
    }
  }
  this->gui->logic(true);
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

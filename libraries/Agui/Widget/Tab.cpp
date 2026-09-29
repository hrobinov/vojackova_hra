#include "Agui/Gui.hpp"
#include "Agui/Widget/Tab.hpp"
#include "Agui/Widget/TabbedPane.hpp"
#include "Agui/Font.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Graphics.hpp"
#include <stdexcept>
#include <Agui/Input.hpp>
#include <Agui/Sound.hpp>

namespace agui
{
  TabStyle Tab::defaultStyle;

  Tab::Tab(const TabStyle* parentStyle)
    : style(this, parentStyle ? parentStyle : &Tab::defaultStyle)
  {}

  Tab::Tab(const std::string& text, const TabStyle* parentStyle)
   : Tab(parentStyle)
  {
    this->setText(text);
  }

  Tab::~Tab()
  {
    if (this->tabPane)
      this->tabPane->removeTab(this);
  }

  void Tab::resizeToContents()
  {
    int width = this->style.getFont()->getTextWidth(this->getText(), RichTextSetting::Enabled);
    int height = this->style.getFont()->getLineHeight();

    if (!this->getBadgeText().empty())
    {
      width += this->style.getBadgeHorizontalSpacing();
      width += this->style.getBadgeFont()->getTextWidth(this->getBadgeText(), RichTextSetting::Disabled);
      width += int(Gui::scale * 4) * 2;
      height = std::max(height, this->style.getBadgeFont()->getLineHeight());
    }

    this->setContentSize(width, height);
  }

  void Tab::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    const agui::Font* font = this->style.getFont();
    const bool hasBadge = !this->getBadgeText().empty();
    const agui::Font* badgeFont = hasBadge ? this->style.getBadgeFont() : nullptr;

    const int contentWidth = this->getContentWidth();
    const int textWidth = font->getTextWidth(this->getText(), RichTextSetting::Enabled);
    const int badgeTextWidth = hasBadge ? badgeFont->getTextWidth(this->getBadgeText(), RichTextSetting::Disabled) : 0;
    const int badgePadding = hasBadge ? int(Gui::scale * 4) : 0;
    const int extraTextWidth = hasBadge ? this->style.getBadgeHorizontalSpacing() : 0;
    const int leftOffset = (contentWidth - textWidth - extraTextWidth - badgePadding - badgeTextWidth - badgePadding) / 2;
    const int textHeight = std::max(font->getLineHeight(), hasBadge ? badgeFont->getLineHeight() : 0);

    paintEvent.graphics()->drawText(Point(leftOffset, (textHeight - font->getLineHeight()) / 2),
                                    this->getText(),
                                    this->getCurrentFontColor(),
                                    font,
                                    RichTextSetting::Enabled);

    if (!this->getBadgeText().empty())
      paintEvent.graphics()->drawText(Point(leftOffset + textWidth + extraTextWidth + badgePadding, (textHeight - badgeFont->getLineHeight()) / 2),
                                      this->getBadgeText(),
                                      this->getCurrentBadgeFontColor(),
                                      badgeFont,
                                      RichTextSetting::Disabled,
                                      HorizontalAlign::Left);
  }

  bool Tab::isSelectedTab() const
  {
    if (this->tabPane)
      return this->tabPane->getSelectedTab() == this;
    return false;
  }

  void Tab::gainedSelection()
  {}

  void Tab::lostSelection()
  {}

  bool Tab::mouseDown(const MouseEvent& mouseEvent)
  {
    super::mouseDown(mouseEvent);
    if (!this->tabPane)
      return false;
    this->tabPane->editSelectedTab(this);
    return false;
  }

  void Tab::setTabPane(TabbedPane* pane)
  {
    this->tabPane = pane;
  }

  void Tab::setText(const std::string& text)
  {
    Widget::setText(text);
    this->resizeToContents();
  }

  void Tab::setText(std::string&& text)
  {
    Widget::setText(std::move(text));
    this->resizeToContents();
  }

  void Tab::setBadgeText(const std::string& text)
  {
    this->setBadgeText(std::string(text));
  }

  void Tab::setBadgeText(std::string&& text)
  {
    if (this->badgeText == text)
      return;

    this->badgeText = std::move(text);
    this->triggerResize();
  }

  bool Tab::keyDown(const KeyEvent& keyEvent)
  {
    if (keyEvent.getExtendedKey() == EXT_KEY_LEFT && this->tabPane)
    {
      if (this->tabPane->getSelectedIndex() > 0)
        this->tabPane->editSelectedTab(tabPane->getSelectedIndex() - 1);
      return true;
    }
    if (keyEvent.getExtendedKey() == EXT_KEY_RIGHT && this->tabPane)
    {
      this->tabPane->editSelectedTab(tabPane->getSelectedIndex() + 1);
      return true;
    }
    return false;
  }

  bool Tab::keyRepeat(const KeyEvent& keyEvent)
  {
    if (keyEvent.getExtendedKey() == EXT_KEY_LEFT && this->tabPane)
    {
      if (this->tabPane->getSelectedIndex() > 0)
        this->tabPane->editSelectedTab(tabPane->getSelectedIndex() - 1);
      return true;
    }
    if (keyEvent.getExtendedKey() == EXT_KEY_RIGHT && this->tabPane)
    {
      this->tabPane->editSelectedTab(tabPane->getSelectedIndex() + 1);
      return true;
    }
    return false;
  }

  void Tab::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->getCurrentImageSet()->base.draw(paintEvent, this->getCurrentDrawingSize(), absolutePosition);

    if (this->getBadgeText().empty())
      return;

    const agui::Font* font = this->style.getFont();
    const agui::Font* badgeFont = this->style.getBadgeFont();

    if (!font || !badgeFont)
      if (agui::Gui::log)
      {
        (*agui::Gui::log)(font ? "no badge font" : "no font");
        (*agui::Gui::log)(this->getParentPathString());
        (*agui::Gui::log)(this->style.getParentPathString());
        std::abort();
      }

    const int contentWidth = this->getContentWidth();
    const int textWidth = font->getTextWidth(this->getText(), RichTextSetting::Enabled);
    const int badgeTextWidth = badgeFont->getTextWidth(this->getBadgeText(), RichTextSetting::Disabled);
    const int badgePadding = int(Gui::scale * 4);
    const int extraTextWidth = this->style.getBadgeHorizontalSpacing();
    const int leftOffset = (contentWidth - textWidth - extraTextWidth - badgePadding - badgeTextWidth - badgePadding) / 2;
    const int lineHeight = badgeFont->getLineHeight() - (badgePadding);
    const int textHeight = std::max(font->getLineHeight(), lineHeight);

    this->getCurrentBadgeImageSet()->base.draw(paintEvent, Rectangle(Point(this->getLeftPadding() + leftOffset + textWidth + extraTextWidth, this->getTopPadding() + (textHeight - lineHeight) / 2),
                                                                     Dimension(badgeTextWidth + (badgePadding * 2), lineHeight)), absolutePosition);
  }

  void Tab::paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->getCurrentImageSet()->glow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void Tab::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    if (paintEvent.graphics()->shadowView)
      this->getCurrentImageSet()->shadow.draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  const ElementImageSet* Tab::getCurrentImageSet() const
  {
    if (!this->isEnabled())
      return this->style.getGraphicalSet(TabStyle::GraphicalSetType::Disabled);

    if (this->isSelectedTab())
    {
      // Hack to properly connect selected tab to the sides of the frame under it in case the frames are on the edge.
      if (this->style.shouldOverrideGraphicsOnEdges())
        if (this->tabPane->isTabFirst(this))
          return this->style.getGraphicalSet(TabStyle::GraphicalSetType::LeftEdgeSelected);
        else if (this->tabPane->isTabLast(this))
          return this->style.getGraphicalSet(TabStyle::GraphicalSetType::RightEdgeSelected);
      if (this->getGui()->input->getInputMethod() == Input::PlayerInputMethod::GameController)
      {
        if (this->getClickStateForRendering() == ClickState::HOVERED)
          return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::GameControllerSelectedHover);
        else
          return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::Selected);
      }
      else
        return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::Selected);
    }

    switch (this->getClickStateForRendering())
    {
      case ClickState::DEFAULT: return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::Default);
      case ClickState::HOVERED: return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::Hover);
      case ClickState::CLICKED: return this->style.getGraphicalSet(agui::TabStyle::GraphicalSetType::Press);
    }

    throw std::logic_error("Unknown click state");
  }

  const ElementImageSet* Tab::getCurrentBadgeImageSet() const
  {
    if (!this->isEnabled())
      return this->style.getBadgeGraphicalSet(TabStyle::GraphicalSetType::Disabled);

    if (this->isSelectedTab())
      return this->style.getBadgeGraphicalSet(TabStyle::GraphicalSetType::Selected);

    switch (this->getClickStateForRendering())
    {
      case ClickState::DEFAULT: return this->style.getBadgeGraphicalSet(TabStyle::GraphicalSetType::Default);
      case ClickState::HOVERED: return this->style.getBadgeGraphicalSet(TabStyle::GraphicalSetType::Hover);
      case ClickState::CLICKED: return this->style.getBadgeGraphicalSet(TabStyle::GraphicalSetType::Press);
    }

    throw std::logic_error("Unknown click state");
  }

  Color Tab::getCurrentFontColor() const
  {
    if (!this->isEnabled())
      return this->style.getDisabledFontColor();

    if (this->isSelectedTab())
      return this->style.getSelectedFontColor();

    return this->style.getDefaultFontColor();
  }

  Color Tab::getCurrentBadgeFontColor() const
  {
    if (!this->isEnabled())
      return this->style.getDisabledBadgeFontColor();

    if (this->isSelectedTab())
      return this->style.getSelectedBadgeFontColor();

    return this->style.getDefaultBadgeFontColor();
  }

  Rectangle Tab::getCurrentDrawingSize() const
  {
    Rectangle result = this->getSizeRectangle();

    if (this->isEnabled() && this->isSelectedTab() && this->style.shouldIncreaseHeightWhenSelected())
      result.height += Gui::scale * 4; // create overlap over the border of the frame under it
    return result;
  }
}

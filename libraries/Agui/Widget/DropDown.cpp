#include "Agui/ElementImageSet.hpp"
#include "Agui/Font.hpp"
#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/Image.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/DropDown.hpp"
#include "Agui/Widget/ListBox.hpp"
#include <Agui/EventDispatchHelper.hpp>
#include <algorithm>

namespace agui
{
  DropDownStyle DropDown::defaultStyle;

  DropDown::DropDown(const DropDownStyle* parentStyle)
    : style(this, parentStyle)
    , listBox(this->style.getListBoxStyle())
  {
    this->setFocusable(true);
    this->setupListBox();
  }

  void DropDown::setupListBox()
  {
    this->listBox.setFocusable(true);
    this->listBox.setVisible(false);
    this->listBox.setSelectedIndex(-1);
    this->listBox.onModalMouseDown(this, [this](const MouseEvent& mouseEvent) { if (mouseEvent.getButton() == MouseButton::LEFT) this->hideDropDown(); });
    this->listBox.onModalMouseUp(this, [this](const MouseEvent& mouseEvent) { if (mouseEvent.getButton() != MouseButton::LEFT) this->hideDropDown(); });
    this->listBox.onKeyDown(this, [this](const KeyEvent& keyEvent) { this->processKey(keyEvent); });
    this->listBox.onKeyRepeat(this, [this](const KeyEvent& keyEvent) { this->processKey(keyEvent); });
    this->listBox.onItemSelect(this, [this](int index)
    {
      if (index == -1)
        return;
      this->hideDropDown();
      int currentIndex = this->getSelectedIndex();
      if (currentIndex == index)
        return;
      this->setSelectedIndex(index);
      this->dispatchItemSelect(this->getSelectedIndex());
    });
  }

  void DropDown::positionListBox()
  {
    this->listBox.style.setConstantWidth(this->getWidth());

    const Point dropdownAbsolutePosition = this->getAbsolutePosition();
    const Point listBoxOffset = this->getListPositionOffset();
    const int dropdownHeight = this->getHeight();
    const int listBoxHeight = this->listBox.getHeight();

    Point listBoxNewLocation = dropdownAbsolutePosition + listBoxOffset + Point(0, dropdownHeight);

    if (const Widget* top = getTopWidget();
        top && listBoxNewLocation.y + listBoxHeight > top->getAbsoluteRectangle().getBottom())
      listBoxNewLocation += Point(0, -(dropdownHeight + listBoxHeight));

    this->listBox.setLocation(listBoxNewLocation);
  }

  void DropDown::onSizeChanged(Dimension originalSize)
  {
    super::onSizeChanged(originalSize);
    this->positionListBox();
  }

  void DropDown::setLocation(const Point& location)
  {
    Widget::setLocation(location);
    this->positionListBox();
  }

  void DropDown::setLocation(int width, int height)
  {
    Widget::setLocation(width, height);
  }

  void DropDown::showDropDown()
  {
    if (!this->dispatchOnBeforeDropdownIsShown())
      return;
    Widget* top = getTopWidget();
    if (top)
    {
      if (this->listBox.getParent())
        this->listBox.getParent()->remove(&this->listBox);
      top->add(&this->listBox);
    }
    this->listBox.setVisible(true);
    this->positionListBox();
    this->listBox.setSelectedIndex(this->getSelectedIndex());
    if (this->getSelectedIndex() <= -1)
      this->listBox.moveToSelection(0, ScrollMode::InView);
    else
    {
      this->listBox.moveToSelection(this->listBox.getItemCount() - 1, ScrollMode::InView);
      this->listBox.moveToSelection(this->getSelectedIndex(), ScrollMode::InView);
    }
    this->listBox.requestModalFocus(ModalFocusPriority::DropDown, true);

    if (const Sound* sound = this->style.getOpenedSound())
      sound->play(1.0);
  }

  void DropDown::hideDropDown()
  {
    this->hideDropdownWithoutFocus();
    this->focus();
  }

  void DropDown::hideDropdownWithoutFocus()
  {
    this->listBox.setVisible(false);
    if (this->listBox.getParent())
      this->listBox.getParent()->remove(&this->listBox);
    this->listBox.releaseModalFocus();
  }

  bool DropDown::mouseDown(const MouseEvent& mouseEvent)
  {
    if (mouseEvent.getButton() != MouseButton::LEFT)
      return false;
    this->showDropDown();
    return true;
  }

  void DropDown::paintComponent(const PaintEvent& paintEvent, const agui::Point&)
  {
    this->drawText(paintEvent);
    int iconWidth = this->style.getIcon()->getWidth() * Gui::scale;
    int iconHeight = this->style.getIcon()->getHeight() * Gui::scale;
    paintEvent.graphics()->drawScaledImage(this->style.getIcon(),
                                           Point(this->getContentWidth() - iconWidth, (this->getContentHeight() - iconHeight) / 2),
                                           Dimension(iconWidth, iconHeight));
  }

  void DropDown::paintBackground(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->paintBackgroundLayer(paintEvent, absolutePosition, ElementImageSet::LayerType::Base);
  }

  void DropDown::paintBackgroundShadow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->paintBackgroundLayer(paintEvent, absolutePosition, ElementImageSet::LayerType::Shadow);
  }

  void DropDown::paintBackgroundGlow(const PaintEvent& paintEvent, const agui::Point& absolutePosition)
  {
    this->paintBackgroundLayer(paintEvent, absolutePosition, ElementImageSet::LayerType::Glow);
  }

  void DropDown::paintBackgroundLayer(const PaintEvent& paintEvent, const agui::Point& absolutePosition, ElementImageSet::LayerType layer)
  {
    this->getCurrentImageSet()->getLayer(layer).draw(paintEvent, this->getSizeRectangle(), absolutePosition);
  }

  void DropDown::reapplySubStyles()
  {
    this->listBox.style.setParent(this->style.getListBoxStyle());
  }

  void DropDown::drawText(const PaintEvent& paintEvent)
  {
    const int extraWidth = this->style.getIcon()->getWidth() + this->style.getSelectorAndTitleSpacing();
    const int width = this->getContentWidth() - extraWidth;
    if (width <= 0)
      return;

    paintEvent.graphics()->pushClippingRect(this, Rectangle(0, 0, width, this->getHeight()), true);
    int textHeight = this->style.getFont()->getLineHeight();
    paintEvent.graphics()->drawText(Point(0, (this->getContentHeight() - textHeight) / 2),
                                    this->getText(),
                                    this->getCurrentFontColor(),
                                    this->style.getFont(),
                                    agui::RichTextSetting::Enabled);
    paintEvent.graphics()->popClippingRect();
  }

  void DropDown::setSelectedIndex(int index)
  {
    this->listBox.setSelectedIndex(index);
    this->updateSelectedIndex();
  }

  void DropDown::editSelectedIndex(int index)
  {
    int val = this->getSelectedIndex();
    this->setSelectedIndex(index);
    if (val != this->getSelectedIndex())
      this->dispatchItemSelect(this->getSelectedIndex());
  }

  void DropDown::setSelectedItem(const std::string& item)
  {
    this->listBox.setSelectedItem(item);
    this->updateSelectedIndex();
  }

  int DropDown::getSelectedIndex() const
  {
    return selectedIndex;
  }

  bool DropDown::processKey(const KeyEvent& keyEvent)
  {
    if (keyEvent.getExtendedKey() == EXT_KEY_DOWN ||
        keyEvent.getExtendedKey() == EXT_KEY_UP ||
        keyEvent.getExtendedKey() == EXT_KEY_HOME ||
        keyEvent.getExtendedKey() == EXT_KEY_END ||
        keyEvent.getKey() == KEY_ENTER)
    {
      this->setSelectedIndex(this->listBox.getSelectedIndex());
      if (keyEvent.getKey() == KEY_ENTER)
        this->hideDropDown();
      return true;
    }
    return false;
  }

  bool DropDown::keyDown(const KeyEvent& keyEvent)
  {
    int val = getSelectedIndex();
    this->handleKeyboard(keyEvent);
    if (val != this->getSelectedIndex())
      this->dispatchItemSelect(this->getSelectedIndex());
    return true;
  }

  void DropDown::handleKeyboard(const KeyEvent& keyEvent)
  {
    if (keyEvent.getKey() == KEY_SPACE)
    {
      this->showDropDown();
      return;
    }
    if (keyEvent.getExtendedKey() == EXT_KEY_DOWN ||
        keyEvent.getExtendedKey() == EXT_KEY_RIGHT)
    {
      if (this->listBox.getSelectedIndex() != this->getSelectedIndex())
      {
        this->setSelectedIndex(this->listBox.getSelectedIndex());
        return;
      }
      this->listBox.setSelectedIndex(this->getSelectedIndex() + 1);
      this->setSelectedIndex(this->listBox.getSelectedIndex());
    }
    else if (keyEvent.getExtendedKey() == EXT_KEY_UP ||
             keyEvent.getExtendedKey() == EXT_KEY_LEFT)
    {
      if (this->listBox.getSelectedIndex() != getSelectedIndex())
      {
        this->setSelectedIndex(this->listBox.getSelectedIndex());
        return;
      }
      if (this->getSelectedIndex() <= 0)
        return;
      this->listBox.setSelectedIndex(getSelectedIndex() - 1);
      this->setSelectedIndex(this->listBox.getSelectedIndex());
    }
  }

  bool DropDown::keyRepeat(const KeyEvent& keyEvent)
  {
    int val = this->getSelectedIndex();
    this->handleKeyboard(keyEvent);
    if (this->getSelectedIndex() != val)
      this->dispatchItemSelect(this->getSelectedIndex());
    return true;
  }

  bool DropDown::isDropDownShowing() const
  {
    return this->listBox.isVisible();
  }

  void DropDown::addItem(std::string&& item)
  {
    this->listBox.addItem(std::move(item));
    this->updateSelectedIndex();
  }

  void DropDown::addItems(std::vector<std::string>&& items)
  {
    this->listBox.addItems(std::move(items));
    this->updateSelectedIndex();
  }

  void DropDown::removeItem(const std::string& item)
  {
    this->listBox.removeItem(item);
    this->updateSelectedIndex();
  }

  void DropDown::removeItemAt(int index)
  {
    this->listBox.removeItemAt(index);
    this->updateSelectedIndex();
  }

  const std::vector<ListBoxItem>& DropDown::getItems() const
  {
    return this->listBox.getItems();
  }

  void DropDown::addItemAt(std::string&& item, int index)
  {
    this->listBox.addItemAt(std::move(item), index);
    this->updateSelectedIndex();
  }

  void DropDown::setItemAt(std::string&& item, int index)
  {
    this->listBox.setItemAt(index, item);
    if (this->selectedIndex == index)
      this->setText(this->listBox.getItemAt(selectedIndex));
  }

  void DropDown::clearItems()
  {
    this->listBox.clearItems();
    this->updateSelectedIndex();
  }

  void DropDown::setItemToolTip(int index, std::string&& text)
  {
    if (index >= this->listBox.getItemCount())
      return;
    this->listBox.setItemToolTip(index, std::move(text));
  }

  const Point& DropDown::getListPositionOffset() const
  {
    return this->listPosOffset;
  }

  void DropDown::setListPositionOffset(const Point& offset)
  {
    this->listPosOffset = offset;
  }

  void DropDown::setListSizePadding(const Dimension& padding)
  {
    this->listSizeIncrease = padding;
  }

  const Dimension& DropDown::getListSizePadding() const
  {
    return listSizeIncrease;
  }

  bool DropDown::mouseEnter(const MouseEvent& mouseEvent)
  {
    agui::Widget::mouseEnter(mouseEvent);
    this->mouseInside = true;
    return true;
  }

  bool DropDown::mouseLeave(const MouseEvent& mouseEvent)
  {
    agui::Widget::mouseLeave(mouseEvent);
    this->mouseInside = false;
    return true;
  }

  bool DropDown::isMouseInside() const
  {
    return this->mouseInside;
  }

  void DropDown::reserveSpaceFor(const std::string& text)
  {
    const Font* font = this->style.getFont();
    int biggestPossibleTextWidth = font->getTextWidth(text, RichTextSetting::Enabled);

    const int extraWidth = this->style.getIcon()->getWidth() + this->style.getSelectorAndTitleSpacing();
    const int width = std::max(extraWidth, this->listBox.itemHolder.verticalScrollBar.getWidth()) +
                      biggestPossibleTextWidth +
                      this->getHorizontalPaddings();
    if (this->style.getMinimalWidth() < width)
      this->style.setMinimalWidth(width);
  }

  void DropDown::updateSelectedIndex()
  {
    this->selectedIndex = this->listBox.getSelectedIndex();
    this->setText(this->listBox.getItemAt(this->selectedIndex));
  }

  const ElementImageSet* DropDown::getCurrentImageSet() const
  {
    if (!this->isEnabled())
      return this->style.getButtonStyle()->getDisabledGraphicalSet();
    if (this->isDropDownShowing())
      return this->style.getButtonStyle()->getClickedGraphicalSet();
    if (this->isMouseInside())
      return this->style.getButtonStyle()->getHoveredGraphicalSet();
    return this->style.getButtonStyle()->getDefaultGraphicalSet();
  }

  const Color& DropDown::getCurrentFontColor() const
  {
    if (!this->isEnabled())
      return this->style.getButtonStyle()->getDisabledFontColor();
    if (this->isDropDownShowing())
      return this->style.getButtonStyle()->getClickedFontColor();
    if (this->isMouseInside())
      return this->style.getButtonStyle()->getHoveredFontColor();
    return this->style.getButtonStyle()->getDefaultFontColor();
  }

  bool DropDown::dispatchOnBeforeDropdownIsShown()
  {
    EventDispatchHelper helper(this, this->actionListeners, Listener::Type::OnBeforeDropdownIsShown);
    bool result = true;
    for (Listener& listener : helper)
      result &= listener.onBeforeDropdownIsShown();
    return helper && result;
  }

  void DropDown::onItemSelect(GenericTargetable* owner, std::function<void(int index)> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnItemSelect);
    this->actionListeners.back().onItemSelect = [callback](int index, bool leftButton){ (void)(leftButton); callback(index); };
  }

  void DropDown::onBeforeDropdownIsShown(GenericTargetable* owner, std::function<bool()> callback)
  {
    this->actionListeners.emplace_back(owner, Listener::Type::OnBeforeDropdownIsShown);
    this->actionListeners.back().onBeforeDropdownIsShown = std::move(callback);
  }

  agui::Widget* DropDown::getListBoxAt(uint32_t index)
  {
    return this->listBox.getButtonAt(index);
  }

  void DropDown::resizeToContents()
  {
    // when the listbox isn't visible, I need to resize it manually to know how big it will be, so I can resize the dropdown
    // button accordingly
    if (!this->listBox.getParent())
      this->listBox.resizeToContentsRecursive();

    // since the listbox items can (and have) different font than the selected dropdown label, I need to make sure that
    // I have enough of space for all the possible selected item text with the dropdown font when using the font of the dropdown.
    const Font* font = this->style.getFont();
    int biggestPossibleTextWidth = font->getTextWidth(this->getText(), RichTextSetting::Enabled);
    for (const ListBoxItem& item : this->listBox.getItems())
      biggestPossibleTextWidth = std::max(biggestPossibleTextWidth, font->getTextWidth(item.button->getText(), RichTextSetting::Enabled));

    const int extraWidth = this->style.getIcon()->getWidth() + this->style.getSelectorAndTitleSpacing();
    const int contentWidth = std::max(extraWidth, this->listBox.itemHolder.verticalScrollBar.getWidth()) +
                             std::max(this->listBox.requiredWidth() - this->listBox.itemHolder.verticalScrollBar.getWidth(), biggestPossibleTextWidth);
    this->setContentSize(contentWidth, font->getLineHeight());
  }

  ToolTip* DropDown::createToolTip()
  {
    if (this->getText().empty())
      return nullptr;

    const Font* font = this->style.getFont();
    const int textWidth = font->getTextWidth(this->getText(), RichTextSetting::Enabled);
    const int extraWidth = this->style.getIcon()->getWidth() + this->style.getSelectorAndTitleSpacing();
    if (textWidth > this->getContentWidth() - extraWidth)
      return new ToolTip(this->getText(), std::string());

    return nullptr;
  }

  int DropDown::getItemCount() const
  {
    return this->listBox.getItemCount();
  }

  const ElementImageSet* DropDown::getBorderImageSet() const
  {
    return this->style.getButtonStyle()->getDefaultGraphicalSet();
  }

  int DropDown::getIndexOf(const std::string& item) const
  {
    return this->listBox.getIndexOf(item);
  }
  std::string DropDown::getItemAt(int index) const
  {
    return this->listBox.getItemAt(index);
  }
}

#pragma once
#include "Agui/Color.hpp"
#include "Agui/Style.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include "Agui/Widget/ListBoxStyle.hpp"

namespace agui
{
  class Font;
  class DropDown;

  class DropDownStyle : public Style
  {
    using super = Style;
  public:
    explicit DropDownStyle(const DropDownStyle* parent = nullptr);
    explicit DropDownStyle(DropDown* relatedWidget, const DropDownStyle* parent = nullptr);
    const DropDownStyle* getParent() const { return static_cast<const DropDownStyle*>(this->parent); }
    virtual void clear() override;

    const Font* getFont() const { return this->getButtonStyle()->getFont(); }
    const ButtonStyle* getButtonStyle() const { return this->getProperty(&DropDownStyle::buttonStyle); }
    ButtonStyle* initButtonStyle() { return this->initProperty(&DropDownStyle::buttonStyle); }
    const Image* getIcon() const { return this->getProperty(&DropDownStyle::icon); }
    void setIcon(const Image* icon) { this->icon = icon; }
    const ListBoxStyle* getListBoxStyle() const { return this->getProperty(&DropDownStyle::listBoxStyle); }
    ListBoxStyle* initListBoxStyle() { return this->initProperty(&DropDownStyle::listBoxStyle); }
    int16_t getSelectorAndTitleSpacing() const { return this->getProperty(&DropDownStyle::selectorAndTitleSpacing); }
    void setSelectorAndTitleSpacing(int16_t selectorAndTitleSpacing) { this->setProperty(&DropDownStyle::selectorAndTitleSpacing, selectorAndTitleSpacing); }
    const Sound* getOpenedSound() const { return this->getPropertyOptional(&DropDownStyle::openedSound); }
    void setOpenedSound(const Sound* leftClickSound) { this->openedSound = leftClickSound; }
    void setOpenedSound(std::nullptr_t);

  private:
    std::unique_ptr<ButtonStyle> buttonStyle;
    const Image* icon = nullptr;
    const Sound* openedSound = nullptr;
    std::unique_ptr<ListBoxStyle> listBoxStyle;
    std::optional<int16_t> selectorAndTitleSpacing;
  };
}

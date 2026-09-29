#pragma once
#include "Agui/Color.hpp"
#include "Agui/ElementImageSet.hpp"
#include "Agui/Style.hpp"
#include "Agui/Sound.hpp"
#include "Agui/Widget/ButtonStyle.hpp"
#include "Agui/Widget/LabelStyle.hpp"
#include <memory>

namespace agui
{
  class Font;
  class Switch;

  class SwitchStyle : public Style
  {
  private:
    using super = Style;
  public:

    explicit SwitchStyle(const SwitchStyle* parent = nullptr);
    explicit SwitchStyle(Switch* relatedWidget, const SwitchStyle* parent = nullptr);
    const SwitchStyle* getParent() const { return static_cast<const SwitchStyle*>(this->parent); }
    virtual void clear() override;

    uint32_t getLeftButtonPosition() const { return this->getProperty(&SwitchStyle::leftButtonPosition); }
    void setLeftButtonPosition(uint32_t leftButtonPosition) { this->leftButtonPosition = leftButtonPosition; }
    uint32_t getMiddleButtonPosition() const { return this->getProperty(&SwitchStyle::middleButtonPosition); }
    void setMiddleButtonPosition(uint32_t middleButtonPosition) { this->middleButtonPosition = middleButtonPosition; }
    uint32_t getRightButtonPosition() const { return this->getProperty(&SwitchStyle::rightButtonPosition); }
    void setRightButtonPosition(uint32_t rightButtonPosition) { this->rightButtonPosition = rightButtonPosition; }
    const Image* getBackgroundDefault() const { return this->getProperty(&SwitchStyle::backgroundDefault); }
    void setBackgroundDefault(const Image* backgroundDefault) { this->backgroundDefault = backgroundDefault; }
    const Image* getBackgroundHover() const { return this->getProperty(&SwitchStyle::backgroundHover); }
    void setBackgroundHover(const Image* backgroundHover) { this->backgroundHover = backgroundHover; }
    const Image* getBackgroundDisabled() const { return this->getProperty(&SwitchStyle::backgroundDisabled); }
    void setBackgroundDisabled(const Image* backgroundDisabled) { this->backgroundDisabled = backgroundDisabled; }
    const LabelStyle* getActiveLabelStyle() const { return this->getProperty(&SwitchStyle::activeLabelStyle); }
    LabelStyle* initActiveLabelStyle() { return this->initProperty(&SwitchStyle::activeLabelStyle); }
    const LabelStyle* getInactiveLabelStyle() const { return this->getProperty(&SwitchStyle::inactiveLabelStyle); }
    LabelStyle* initInactiveLabelStyle() { return this->initProperty(&SwitchStyle::inactiveLabelStyle); }
    const ButtonStyle* getButtonStyle() const { return this->getProperty(&SwitchStyle::buttonStyle); }
    ButtonStyle* initButtonStyle() { return this->initProperty(&SwitchStyle::buttonStyle); }

  private:
    std::optional<uint32_t> leftButtonPosition; // relative offset from the background
    std::optional<uint32_t> middleButtonPosition;
    std::optional<uint32_t> rightButtonPosition;
    const Image* backgroundDefault = nullptr;
    const Image* backgroundHover = nullptr;
    const Image* backgroundDisabled = nullptr;
    std::unique_ptr<LabelStyle> activeLabelStyle;
    std::unique_ptr<LabelStyle> inactiveLabelStyle;
    std::unique_ptr<ButtonStyle> buttonStyle;
  };
}

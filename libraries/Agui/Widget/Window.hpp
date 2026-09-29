#pragma once
#include "Agui/Widget/Frame.hpp"
#include "Agui/Widget/FrameStyle.hpp"
#include "Agui/Widget/HorizontalFlow.hpp"
#include "Agui/GuiDirection.hpp"
#include <memory>
namespace agui { class ElementImageSet; }

namespace agui
{
  /** Movable and resizable Frame / Window. */
  class Window : public Frame
  {
    using super = Frame;
  public:
    enum class HeightRule
    {
      MaxScreenHeight,
      MaxScreenHeightWithExtraSpace,
      ReasonableMinimumAndMaxScreenWithExtraSpace,
      ConstantScreenHeightWithExtraSpace,
      ConstantScreenHeight,
    };

    // Positioning when anchorToPointOrder or anchorToWidgetOrder is set.
    // First part is about the reference, second is the position towards it
    enum class AnchorType
    {
      CenterToCenter, // widget centers overlap
      CenterUnderCenter,
      BottomLeftToRight, // the window's top-left corner is anchored to the anchor's bottom-left corner if there is enough vertical space
      BottomRightToLeft, // the window's top-right corner is anchored to the anchor's bottom-right corner
      CenterLeftToLeft, // centered vertically and going left from the anchor's left side
      TopLeftToRight, // the window's bottom-left corner is anchored to the anchor's top-left corner if there is enough vertical space.
      TopLeftToLeft, // the window's top-right corner is anchored to the anchor's top-left corner
      TopRightToRight,
    };

    Window(GuiDirection direction, const FrameStyle* parentStyle = nullptr, HeightRule heightRule = HeightRule::MaxScreenHeightWithExtraSpace, const std::string& title = "");
    Window(GuiDirection direction, HeightRule heightRule);
    Window(GuiDirection direction, const std::string& title);

    virtual void setContentSize(int width, int height) override;
    virtual void resizeToContents() override;

    virtual bool mouseDown(const MouseEvent& mouseEvent) override;
    virtual bool mouseDrag(const MouseEvent& mouseEvent) override;
    virtual bool isMovable() const; // if dragging the caption bar (top of the frame) will result in moving the Frame.
    virtual bool mouseUp(const MouseEvent& mouseEvent) override;
    virtual bool dragOnlyByLeftMouseButton() const override { return true; }
    void dispatchCentered();

    virtual void displaySizeChanged() override;
    void ensureWholeWindowIsOnScreen();
    virtual bool previousMod() { return false; }
    void initAsMain(bool autoCenter = true);
    virtual void clear() override;
    void anchorToPoint(agui::Point point, AnchorType anchorType = AnchorType::CenterToCenter);
    void anchorToPoint(const agui::Widget* widget, AnchorType anchorType = AnchorType::CenterToCenter); // takes a point from the widget as reference
    void anchorToPoint(const agui::Widget& widget, AnchorType anchorType = AnchorType::CenterToCenter);
    void anchorToWidget(agui::Widget& widget, AnchorType anchorType = AnchorType::BottomLeftToRight); // keeps the widget as reference
    void positionToWidget(const agui::Widget* widget, AnchorType anchorType = AnchorType::CenterToCenter); // one time positioning
    void anchor();
    bool checkAnchor();
    void autoCenter() { this->autoCenterEnabled = true; this->autoCenterOrdered = true; }
    void forceAutoCenter() { this->autoCenter(); this->triggerResize(); }
    void disableAutoCenter() { this->autoCenterEnabled = false; this->autoCenterOrdered = false; }
    Window* getParentWindow();
    void checkFullscreenInParent();
    void setFullScreen(bool value);

    static uint32_t getMinimumDifferenceBetweenDisplayHeightAndWindowContentsHeight();

  private:
    void updateMaximumSize();
    void center(); // Centers the window in the middle of the screen.
    void anchorToPointInternal();
    void anchorToWidgetInternal();

  public:
    bool isMain = false;
    bool autoEnsureWholeWindowIsVisible = true;
  protected:
    bool fullScreen = false;
  private:
    Point dragPoint;
    bool moving = false;
    bool autoCenterEnabled = false;
    bool autoCenterOrdered = false;
    agui::Point anchorToPointOrder = agui::Point::emptyPoint();
    GenericTargeter<Widget> anchorToWidgetOrder;
    AnchorType anchorType = AnchorType::CenterToCenter;
    HeightRule heightRule;
  };
}

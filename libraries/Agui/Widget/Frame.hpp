#pragma once
#include "Agui/Widget.hpp"
#include "Agui/Widget/Filler.hpp"
#include "Agui/Widget/FrameStyle.hpp"
#include "Agui/Widget/HorizontalFlow.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/GuiDirection.hpp"
#include <memory>
namespace agui { class ElementImageSet; }

namespace agui
{
  class Frame : public Widget
  {
    using super = Widget;
  public:
    Frame(GuiDirection direction, const FrameStyle* parentStyle = nullptr);
    Frame(GuiDirection direction, const FrameStyle* parentStyle, const std::string& title);
    Frame(GuiDirection direction, const std::string& title);
    virtual ~Frame() = default;
    Frame& operator<<(const Pusher& pusher);
    Frame& operator<<(const VerticalPusher& pusher) { Widget::operator<<(pusher); return *this; }
    Frame& operator<<(Widget& other) { this->add(&other); return *this; }
    Frame& operator<<(Widget* other) { this->add(other); return *this; }
    template<class T> requires std::is_base_of_v<Widget, T>
    Frame& operator<<(std::unique_ptr<T>& other) { this->add(other.get()); return *this; }
    Frame& operator<<(const Empty& empty) { Widget::operator<<(empty); return *this; }
    Frame& operator<<(const HorizontalPusher& hPusher) { Widget::operator<<(hPusher); return *this; }

    virtual Style* getStyle() override { return &this->style; }
    /** Flags the content pane's public chilagXen in addition to its Frame children. */
    virtual void flagAllChildrenForDestruction() override;
    virtual Widget* getContentHolder();
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()) override;
    virtual int maximumVerticalSquashSize() const override;
    virtual int maximumHorizontalSquashSize() const override;
    virtual bool shouldClipRendering() const override { return false; }
    virtual Widget& dontShrinkInReactionToSetSize() override;

    virtual void reapplySubStyles() override;
    virtual bool empty() const override;
    virtual void add(Widget* widget) override;
    void addBefore(Widget* toAdd, const Widget* before);
    virtual void insert(Widget* widget, uint32_t index) override;
    virtual void addFront(Widget* widget) override;
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False) override;
    virtual void swapChildren(size_t index1, size_t index2) override;
    virtual void resizeToContents() override;
    virtual int getTopBorder() const override;
    virtual int getRightBorder() const override;
    virtual int getBottomBorder() const override;
    virtual int getLeftBorder() const override;
    Frame& stretchHorizontally() { Widget::stretchHorizontally(); this->layout->stretchHorizontally(); return *this; } // to keep the Frame return type
    Frame& stretchVertically() { Widget::stretchVertically(); this->layout->stretchVertically(); return *this; } // to keep the Frame return type
    Frame& setHorizontalAlign(agui::HorizontalAlign align) { Widget::setHorizontalAlign(align); this->layout->setHorizontalAlign(align); return *this; }
    Frame& setVerticalAlign(agui::VerticalAlign align) { Widget::setVerticalAlign(align); this->layout->setVerticalAlign(align); return *this; }
    virtual void displaySizeChanged() override;

    virtual const ElementImageSet* getBorderImageSet() const override;
    virtual TransparentValue isTransparent() const override;

    virtual void clear() override;

    void addToHeaderFlow(Widget* widget);
    void addToHeaderFlow(Pusher);
    void addToHeaderFlowLeft(Widget* widget);
    void addToHeaderFlowBeforeLast(Widget* widget);
    void updateHeaderFillerVisibility();
    virtual void setText(const std::string& text) override;
    virtual void setText(std::string&& text) override;

    virtual Frame& setDragTarget(Window* dragTarget); // Sets whether or not dragging the the frame will result in moving the Frame.
    virtual Window* getDragTarget() override;
    virtual const Window* getDragTarget() const override;

    virtual void setSpacing(int spacing); // sets vertical or horizontal spacing depending on direction
    VerticalFlow* getVerticalFlow();
    HorizontalFlow* getHorizontalFlow();
    HorizontalFlow* getHeaderFillerFlow() { return &this->headerFlow; }
    Filler* getFiller() { return &this->filler; }

  protected:
    virtual void paintBackgroundShadow(const PaintEvent&, const Point& absolutePosition) override;
    virtual void paintBackground(const PaintEvent&, const Point& absolutePosition) override;
    virtual void paintBackgroundGlow(const PaintEvent&, const Point& absolutePosition) override;
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition) override;
  public:
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
    virtual void recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true) override;
  protected:

    virtual void resizeContainer(SetSizeInfo setSizeInfo);
    void updateHeaderFlowSizeAndLocation();
    int getTopPartHeight() const;

  private:
    void updateFillerRightMargin();
    int requiredTopPartWidth();
    void updateHeaderFlowVisibility();

  public:
    static FrameStyle defaultStyle;

    FrameStyle style;
    agui::Layout* layout;

  private:
    GuiDirection direction;
    Window* dragTarget = nullptr;
  protected:
    HorizontalFlow headerFlow;
    Filler filler;
    static const int EXTRA_RIGHT_MARGIN_WHEN_SOMETHING_IS_NEXT_TO_FILLER = 4;
  public:
    Label title;
    bool forceTransparent = false;
  };
}

#pragma once
#include <Agui/AlignmentEnum.hpp>
#include <Agui/AreaAlignmentEnum.hpp>
#include <Agui/Color.hpp>
#include <Agui/CursorProvider.hpp>
#include <Agui/GameControllerInteraction.hpp>
#include <Agui/GenericTargeter.hpp>
#include <Agui/GuiDirection.hpp>
#include <Agui/KeyEvent.hpp>
#include <Agui/Listener.hpp>
#include <Agui/ModalFocusPriority.hpp>
#include <Agui/MouseButton.hpp>
#include <Agui/MouseEvent.hpp>
#include <Agui/Rectangle.hpp>
#include <Agui/SideEnum.hpp>
#include <Agui/LowercaseString.hpp>
#include <Agui/NamedBool.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <stack>
#include <vector>

#ifdef DEBUG
#include <unordered_set>
#endif

namespace agui
{
  class ScrollPane;
  class TableWithSelection;
  class Slider;
  class Button;
  class ButtonStyle;
  class CheckBox;
  class Clickable;
  class ElementImageSet;
  class EmptyWidget;
  class EmptyWidgetStyle;
  class FocusManager;
  class Font;
  class Frame;
  class FrameStyle;
  class Graphics;
  class Gui;
  class ImageStyle;
  class HorizontalFlow;
  class HorizontalFlowStyle;
  class HorizontalLine;
  class Image;
  class ImageWidget;
  class KeyEvent;
  class Label;
  class LabelStyle;
  class LineStyle;
  class ListBox;
  class PaintEvent;
  class Pusher;
  class HorizontalPusher;
  class VerticalPusher;
  class ScrollPaneStyle;
  class Style;
  class Tab;
  class TabStyle;
  class Table;
  class TableStyle;
  class ToolTip;
  class ToolTipCreatorBase;
  class VerticalFlow;
  class VerticalFlowStyle;
  class VerticalLine;
  class VerticalScrollPane;
  class Widget;
  class Window;
  struct TextInputInfo;
  enum class RichTextSetting : uint8_t;
}

namespace agui
{
  struct TableIndex
  {
    constexpr TableIndex() = default;
    explicit constexpr TableIndex(uint32_t value)
      : hasValue(value != uint32_t(-1))
      , value(value)
    {}

    bool hasValue = false;
    uint32_t value = uint32_t(-1);
  };

  using WidgetArray = std::vector<Widget*>;

  class Empty{};
  static constexpr Empty empty; // Signal that adds empty widget with auto-destruction when removed
  class HFlow
  {
  public:
    HorizontalFlow& operator()(VerticalAlign align) const;
    HorizontalFlow& operator()(HorizontalAlign align) const;
    HorizontalFlow& operator()(AreaAlign align) const;
    HorizontalFlow& operator()(const HorizontalFlowStyle* style) const;
    HorizontalFlow& operator<<(Widget& widget) const;
    HorizontalFlow& operator<<(Widget* widget) const;
    HorizontalFlow& operator<<(const Pusher& pusher) const;
    template<class T> requires std::is_base_of_v<Widget, T>
    HorizontalFlow& operator<<(std::unique_ptr<T>& other) { return *this << *other; }
    HorizontalFlow& centerVertically() const;
    HorizontalFlow& stretchHorizontally() const;
    HorizontalFlow& dontShrinkInReactionToSetSize() const;
    operator HorizontalFlow&() const;
  };
  static const HFlow hFlow; // Signal that adds horizontal flow with auto-destruction when removed

  class VFlow
  {
  public:
    VerticalFlow& operator()(VerticalAlign align) const;
    VerticalFlow& operator()(AreaAlign align) const;
    VerticalFlow& operator()(const VerticalFlowStyle* parentStyle) const;
    VerticalFlow& operator<<(Widget& widget) const;
    VerticalFlow& dontDecreaseWidth() const;
    VerticalFlow& dontDecreaseHeight() const;
    VerticalFlow& dontDecreaseSize() const;
    VerticalFlow& centerVertically() const;
    VerticalFlow& stretchHorizontally() const;
    VerticalFlow& dontShrinkInReactionToSetSize() const;
    template<class T> requires std::is_base_of_v<Widget, T>
    VerticalFlow& operator<<(std::unique_ptr<T>& other) { return *this << *other; }
    operator VerticalFlow&() const;
  };
  static const VFlow vFlow; // Signal that adds vertical flow with auto-destruction when removed

  class VScroll
  {
  public:
    VerticalScrollPane& operator<<(Widget& widget) const;
    VerticalScrollPane& operator()(const ScrollPaneStyle* parentStyle) const;
    operator VerticalScrollPane&() const;
    VerticalScrollPane& stretchHorizontally() const;
  };
  static const VScroll vScroll; // Signal that adds vertical scroll pane with auto-destruction when removed

  enum class Change
  {
    Normal,
    Stretching,
    Squashing,
    Nothing
  };

  class HorizontalLineHelper
  {
  public:
    HorizontalLine& operator()(const LineStyle* parentStyle = nullptr);
    operator HorizontalLine&() const;
  };
  inline HorizontalLineHelper horizontalLine;

  using TabbedIn = NamedBool<class TabbedInTag>;
  using KeepWidgetAlive = NamedBool<class KeepWidgetAliveTag>;

  using ReactionToSetSize = NamedBool<class ReactionToSetSizeTag>;

  class SetSizeInfo
  {
  public:
    constexpr SetSizeInfo() = default;
    constexpr SetSizeInfo(ReactionToSetSize reactionToSetSize)
      : reactionToSetSize(reactionToSetSize) {}
    constexpr SetSizeInfo(Change horizontal, Change vertical)
      : horizontal(horizontal)
      , vertical(vertical)
    {}
    bool squashing() const { return this->horizontal == Change::Squashing || this->vertical == Change::Squashing; }

    Change horizontal = Change::Normal, vertical = Change::Normal;
    int maximalWidth = 0;
    ReactionToSetSize reactionToSetSize = ReactionToSetSize::True;
  };

  enum class TransparentValue
  {
    No, DependsOnChildren, Yes
  };

  TransparentValue transparentOpaqueMask(TransparentValue value, TransparentValue mask);

  /** Abstract base class for all widgets in Agui
   *
   * Size concept
   *   - Size is the total size of the widget with all the borders and paddings included, this is how much space will the widget
   *     take in some layout etc.
   *   - ContentSize is the size available for the inner content of the widget. It is smaller than size as paddings and graphical
   *     border needs to be taken into account.
   *
   * Position concept
   *   - Position of widget is always relative to its parent widget. This means, that moving parent widget doesn't require to
   *     update positions of inner widgets.
   *   - Position of Point(0, 0), is the left top corner of the content rectangle, so this position has paddings already applied.
   *
   * Padding concept
   *   - Paddings have two parts padding specified in Style and for some elements (buttons, frames) the graphic border of the widget
   *   - Methods to get paddings always return both of these values summed.
   *
   * Order concept
   *   - Widgets can generally intersect. In that case, the order of widgets in the children container determines which is on top
   *     The drawing is done from first to last, so the last widget is on top. All other logic, like determining widget under mouse
   *     and similar respect the visual representation.
   *
   * Visibility concept
   *   - There are two kinds of invisibility.
   *   - The first one is achieved by calling style.setVisible(false/true), which makes it as the widget doesn't even exist.
   *     Not only it is invisible, but it is ignored when layouting etc.
   *   - The second one is achieved by calling setRender(false/true), which makes the widget also invisible and not hoverable
   *     but its space will be still respected in the layouts. This is useful when we want to hide something without the layout
   *     "jumping around".
   *
   */
  class Widget : public GenericTargetable
  {
  public:
    class WidgetsToDestroy
    {
    public:
      ~WidgetsToDestroy();
      void destroyWidgets();
      std::vector<Widget*> widgets;
    } static widgetsToDestroy;

#ifdef DEBUG
    Widget()
    {
      Widget::widgets.insert(this);
    }
#else
    Widget() = default;
#endif

    class visible_iterator
    {
    public:
      visible_iterator(Widget& parent) : parent(&parent), position(parent.begin()) { this->nextValid(); }
      visible_iterator& operator*() { return *this; }
      void operator++() { ++this->position; this->nextValid(); }
      void operator=(const visible_iterator& other) { this->parent = other.parent; this->position = other.position; }
      bool operator!=(const visible_iterator& other) const { return this->position != other.position; }
      bool operator!=(const WidgetArray::iterator& other) const { return this->position != other; }
      Widget& widget() { return **position; }
      operator Widget*() const { return *position; }
    private:

      void nextValid() { while (this->position != this->parent->end() && !this->widget().isVisible()) ++this->position; }
    private:
      Widget* parent;
      WidgetArray::iterator position;
    };

    class const_visible_iterator
    {
    public:
      const_visible_iterator(const Widget& parent) : parent(&parent), position(parent.begin()) { this->nextValid(); }
      const_visible_iterator& operator*() { return *this; }
      void operator++() { ++this->position; this->nextValid(); }
      void operator=(const const_visible_iterator& other) { this->parent = other.parent; this->position = other.position; }
      bool operator!=(const const_visible_iterator& other) const { return this->position != other.position; }
      bool operator!=(const WidgetArray::const_iterator& other) const { return this->position != other; }
      const Widget& widget() const { return **position; }
      operator const Widget*() const { return *position; }
    private:

      void nextValid() { while (this->position != this->parent->end() && !this->widget().isVisible()) ++this->position; }
    private:
      const Widget* parent;
      WidgetArray::const_iterator position;
    };

    class VisibleChildren
    {
    public:
      VisibleChildren(Widget* widget) : widget(*widget) {}
      visible_iterator begin() { return visible_iterator(this->widget); }
      WidgetArray::iterator end() { return this->widget.end(); }

      Widget& widget;
    };

    class ConstVisibleChildren
    {
    public:
      ConstVisibleChildren(const Widget* widget) : widget(*widget) {}
      const_visible_iterator begin() { return const_visible_iterator(this->widget); }
      WidgetArray::const_iterator end() { return this->widget.end(); }

      const Widget& widget;
    };

    virtual ~Widget();
    Widget(const Widget& other) = delete;
    Widget(const Widget&& other) = delete;
    Widget& operator=(const Widget& other) = delete;
    Widget& operator=(const Widget&& other) = delete;

    virtual void recalculateClippingRect() const;
    Rectangle getClippingRect() const;
  private:
    /** Generates a new mouse event where the source is the widget. Used when dispatching mouse listener events. */
    MouseEvent addSourceToMouseEvent(const MouseEvent& mouseEvent);
    /** Generates a new keyboard event where the source is the widget. Used when dispatching keyboard listener events. */
    KeyEvent addSourceToKeyEvent(const KeyEvent& keyEvent);
    /** Brings this child widget to the front. This affects render order of children and private children. */
    void setFrontWidget(Widget* widget);
    /** Sends this child widget to the back. This affects render order of children and private children. */
    void setBackWidget(Widget* widget);
    void handleRemoval(); // handles tooltip removal, widget under mouse and forceHover if needed

    bool removeChildInternal(Widget* widget, KeepWidgetAlive keepWidgetAlive);
    bool removePrivateChildInternal(Widget* widget, KeepWidgetAlive keepWidgetAlive);
    void addUnchecked(Widget* widget);

  protected:
    /** Paints the interior of the widget after the background has been painted.
     * painting is relative to the top left margins.
     *
     * This means drawing at 0,0 will draw at LEFT_MARGIN, TOP_MARGIN
     * relative to where the widget's 0,0 is.
     *
     * The clipping rectangle does not permit you to draw outside of the
     * margins, and you should not do so, although if you insist,
     * you can call paintEvent.graphics()->popClippingRect().
     *
     * Must be implemented.
     * @param  paintEvent Object to paint with. */
    virtual void paintComponent(const PaintEvent&, const Point& absolutePosition) { (void)(absolutePosition); }
    /** Paints the background of the widget.
     * painting is relative to the widget's top left corner.
     *
     * This means drawing at 0,0 will draw
     * relative to where the widget's 0,0 is.
     *
     * The clipping rectangle does not permit you to draw outside of the
     * widget's area, and you should not do so, although if you insist,
     * you can call paintEvent.graphics()->popClippingRect().
     * @param  paintEvent Object to paint with. */
    virtual void paintBackgroundShadow(const PaintEvent&, const Point& absolutePosition) { (void)(absolutePosition); }
    virtual void paintBackground(const PaintEvent&, const Point& absolutePosition) { (void)(absolutePosition); }
    virtual void paintBackgroundGlow(const PaintEvent&, const Point& absolutePosition) { (void)(absolutePosition); }
    Widget* getFocusedWidget() const; // @return The Gui's focused widget or nullptr if this widget is not part of a Gui.
    virtual void recursivePaintChildrenInternal(bool enabled, Graphics* graphicsContext, const Point& absolutePosition);
    /** @return A boolean indicating if the parameter widget is a child of this widget.
     * @param  widget The widget you would like to check for. */
    virtual bool containsPrivateChild(Widget* widget) const;
    /** @return The index of the private child in the internal private widget std::vector, -1 if not found */
    virtual int getPrivateChildIndex(Widget* widget) const;
    /** Adds this widget as a private widget.
     * Private widgets are intended to help make a more complex widget. For example, a scroll bar has 3 private widgets.
     * It is also convenient if the user wants to clear the children. Private children are rendered before children. */
    virtual void addPrivateChild(Widget* widget);
    virtual void removePrivateChild(Widget* widget);
    /** Dispatches the the event to all of the widget's action listeners. Returns false if 'this' widget was deleted. */
    bool dispatchConfirm(const agui::KeyEvent& keyEvent);
    bool dispatchItemSelect(int index, bool leftClick = true);
    bool dispatchItemSelectConfirm(int index);
    bool dispatchItemDoubleClick(int index);
    bool dispatchSliderMove(double value);
    bool dispatchTextEdit();
    virtual void onSizeChanged(Dimension originalSize);
    Point createAlignedPosition(AreaAlign alignment, const Rectangle& parentRect, const Dimension& childSize) const;
    void handleChildRemoved(Widget* child, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False);
    const Widget* visibleBack() const; // last visible child
  public:
    virtual bool isTextBox() const { return false; }
    virtual bool isSelectedInfo() const { return false; }

    virtual void reapplySubStyles() {}
    virtual void afterResizeIsSettledAction() {}
    void dispatchMouseMove(const MouseEvent& mouseEvent);
    void dispatchMouseLeave(const MouseEvent& mouseEvent);
    void dispatchMouseEnter(const MouseEvent& mouseEvent);
    void dispatchMouseWheelDown(const MouseEvent& mouseEvent);
    void dispatchMouseWheelUp(const MouseEvent& mouseEvent);
    void dispatchMouseWheelLeft(const MouseEvent& mouseEvent);
    void dispatchMouseWheelRight(const MouseEvent& mouseEvent);
    void dispatchMouseDrag(const MouseEvent& mouseEvent);
    void dispatchModalMouseDown(const MouseEvent& mouseEvent);
    void dispatchMouseDown(const MouseEvent& mouseEvent);
    void dispatchMouseUp(const MouseEvent& mouseEvent);
    void dispatchClick(const MouseEvent& mouseEvent);
    void dispatchToggle(bool leftClick = true);
    void dispatchDoubleClick(const MouseEvent& mouseEvent);
    void dispatchMouseHover(const MouseEvent& mouseEvent);
    void dispatchItemDrag(int fromIndex, int toIndex);

    void applySizeRestrictions(const Style& style); /* called internally by the style, shouldn't be called manually */
    void applySizeRestrictionsInternal(const Style& style); // doesn't call the onChangedSize event
    bool applySizeRestrictionsInternalWithoutTriggerResize(const Style& style); // doesn't call the triggerResize, returns true if size was changed

    /** This is default style for widgets that don't have style, like topContainer, The default style is 0 paddings, left unset. */
    const Style* getStyle() const
    {
      if (Style* style = const_cast<Widget*>(this)->getStyle())
        return style;
      return &Widget::defaultWidgetStyle;
    }
    virtual Style* getStyle() { return nullptr; }
    Widget& setStyle(const Style* style);
    virtual bool empty() const { return this->children.empty(); }
    bool _dispatchKeyboardListenerEvent(KeyEvent::KeyboardEventEnum event, const KeyEvent& keyEvent);
    virtual void recursivePaintShadows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true);
    void recursivePaintChildren(bool enabled, Graphics* graphicsContext, const Point& absolutePosition);
    virtual void recursivePaintGlows(bool enabled, Graphics* graphicsContext, const Point& absolutePosition, bool forceInFrame = false, bool includeThis = true);
    virtual bool handlesMouseWheel(bool /*isShiftDown*/) const { return false; }
    /** If it is flagged, the Gui it belongs to will delete it in the next logic loop
     * unless it was not part of a Gui when it was flagged or a flag in the Gui
     * has been set indicating that the Gui's stack of flagged widgets must be manually popped.
     * @return A boolean indicating if this widget is flagged for destruction. */
    virtual bool isFlaggedForDestruction() const;
    virtual void flagForDestruction(); // Will flag this widget for destruction on next logic loop.
    /** Will mark the widget as flagged for destruction, but it will not take responsibility for the destruction of the widget. */
    virtual void flagForDestructionQuiet() { this->usageBitMask |= FLAGGED_FOR_DESTRUCTION; }
    /** Will flag this widget's public children for destruction.
     * Will not flag its private children for destruction. */
    virtual void flagChildrenForDestruction();
    /** Will flag this widget's public children for destruction and recursively all of their public children.
     * Will not flag its private children for destruction nor those of the children. */
    virtual void flagAllChildrenForDestruction();
    virtual void sendToTop(); // If the top most widget can be found, this widget will be added as a child of the top.
    virtual void clear(); // Clears and removes all public children from this widget.
    // Goes through the parents and makes sure that the position stays visible in the scroll pane
    // Scroll Pane overrides this functions and stops the propagation.
    virtual void keepVerticallyVisibleInScrollPane(int verticalPosition, int margin); // margin to the end of the scroll area

    WidgetArray::iterator begin() { return this->children.begin(); }
    WidgetArray::iterator end() { return this->children.end(); }
    WidgetArray::const_iterator begin() const { return this->children.cbegin(); }
    WidgetArray::const_iterator end() const { return this->children.cend(); }
    WidgetArray::reverse_iterator rbegin() { return this->children.rbegin(); }
    WidgetArray::reverse_iterator rend() { return this->children.rend(); }
    WidgetArray::const_reverse_iterator rbegin() const { return this->children.crbegin(); }
    WidgetArray::const_reverse_iterator rend() const { return this->children.crend(); }
    WidgetArray& getChildren() { return this->children; }
    const WidgetArray& getChildren() const { return this->children; }
    WidgetArray& getPrivateChildren() { return this->privateChildren; }
    const WidgetArray& getPrivateChildren() const { return this->privateChildren; }

    uint32_t getPrivateChildCount() const;
    Widget* getPrivateChildAt(uint32_t index) const;
    Widget* getPrivateChildAt(int index) const = delete; // signed index makes no sense
    virtual void add(Widget* widget);
    virtual void insert(Widget* widget, uint32_t index);
    virtual void addFront(Widget* widget);
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False);
    void removeFromParent(KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False);
    /** We use move instead of addBefore to avoid problems with overriden add functions in the existing codebase. */
    void moveWidgetBefore(Widget* widget, Widget* moveBeforeWidget);
    virtual void swapChildren(size_t index1, size_t index2);
    bool containsChildWidget(Widget* widget) const; // @return @c true if the specified widget is direct child
    /** @return The index of the parameter widget in this widget's public children std::vector.
     * @return -1 if not found. */
    int getChildWidgetIndex(const Widget* widget) const;
    const Widget* getNextSibling() const;
    uint32_t getChildCount() const;
    virtual bool mouseClick(const MouseEvent& mouseEvent); // The mouse was pressed down on the same widget it was pressed up on.
    virtual bool keyDown(const KeyEvent& keyEvent); // Called when a key has been pressed down.
    virtual bool keyRepeat(const KeyEvent& keyEvent); // Called when a key press is repeated.
    virtual bool keyUp(const KeyEvent& keyEvent); // Called when a key is released.
    virtual bool mouseDown(const MouseEvent&) { return false; } // Called when the mouse is pressed down.
    /** The modal widget receives this when the mouse is pressed down on a widget other than
     * the modal widget or any of its children and their descendants. */
    virtual bool modalMouseDown(const MouseEvent& mouseEvent);
    /** The modal widget receives this when the mouse is released on a widget other than
     * the modal widget or any of its children and their descendants. */
    virtual bool modalMouseUp(const MouseEvent& mouseEvent);
    virtual bool mouseDoubleClick(const MouseEvent& mouseEvent); // Called when the mouse is double clicked within the threshold set in the Gui.
    virtual bool mouseMove(const MouseEvent& mouseEvent); // Called when the mouse is moved.
    virtual bool mouseDrag(const MouseEvent& mouseEvent); // Called when the mouse is moved while pressed.
    virtual bool mouseUp(const MouseEvent&) { return false; } // Called when the mouse is released.
    /** Called when the vertical mouse wheel is changed positively.
     * @param mouseEvent Information about the mouse event. */
    virtual bool mouseWheelUp(const MouseEvent& mouseEvent);
    virtual bool mouseWheelDown(const MouseEvent& mouseEvent); // Called when the vertical mouse wheel is changed negatively.
    virtual bool mouseWheelLeft(const MouseEvent& mouseEvent); // Called when the horizontal mouse wheel is changed positively.
    virtual bool mouseWheelRight(const MouseEvent& mouseEvent); // Called when the horizontal mouse wheel is changed negatively.
    virtual bool mouseEnter(const MouseEvent&) { return false; } // Called when the mouse enters the widget.
    virtual bool mouseLeave(const MouseEvent& mouseEvent); // Called when the mouse leaves the widget.
    virtual bool mouseHover(const MouseEvent& mouseEvent); //Called when the mouse has been idle over this widget for a certain time.
    virtual bool itemDrag(uint32_t from, uint32_t to) { (void) from; (void) to; return false; } // Called when a child widget is dragged from one index to another.
    virtual void requestModalFocus(ModalFocusPriority priority, bool isDropDownListBox = false);
    virtual void releaseModalFocus();
    virtual void requestTopModalFocusOrNone(); // Requests modal focus equal to or higher than the top modal widget or nothing if there is no modal widget.
    virtual bool consumesKeyWhenFocused(KeyEnum) const { return false; } //consume the key when this or a child widget is focused
    bool isModal() const;

    Widget* getChildAt(uint32_t index) const; // @return The public child at the specified index or nullptr if not found.
    Widget* getChildAt(int32_t) const = delete; // signed index makes no sense
    Widget* getParent() const; // @return This widget's parent or nullptr if the widget has no parent.
    /** If this widget has a parent but the parent is not in some way part of a Gui
     * then the returned widget may not be the top most one of the Gui.
     * @return The top most widget or nullptr if this widget has no parent. */
    Widget* getTopWidget() const;
    /** @return The Gui associated with the top most widget.
     * Will return nullptr if the top most widget cannot be found. */
    Gui* getGui() const;
    /** @return A boolean indicating if this widget can receive focus and modal focus. */
    virtual bool isFocusable() const;
    /** When a widget is disabled it will not receive mouse or keyboard input.
     * It will also set the enabled flag in PaintEvent to false.
     * If this widget is disabled, its children inherently cannot receive input.
     * @return A boolean indicating if this widget is enabled. */
    virtual bool isEnabled() const;
    /** When a widget is not visible, it will not receive input, nor be rendered.
     * If this widget is not visible, its children inherently will not be visible.
     * @return A boolean indicating if this widget is visible. */
    bool isVisible() const;
    void setVisible(bool visible);
    bool setVisibleSilent(bool visible); // don't call trigger resize. This is used for scrollbar updates inside resizing of scrollable objects
    bool hasVisibleChild() const;
    Widget& shrinkInReactionToSetSize() { this->usageBitMask |= SHRINK_IN_REACTION_TO_SET_SIZE; return *this; }
    virtual Widget& dontShrinkInReactionToSetSize() { this->usageBitMask &= ~SHRINK_IN_REACTION_TO_SET_SIZE; return *this; }
    Widget& setShrinkingInReactionToSetSize(bool value) { if (value) this->shrinkInReactionToSetSize(); else this->dontShrinkInReactionToSetSize(); return *this; }
    bool canShrinkInReactionToSetSize() const { return (this->usageBitMask & SHRINK_IN_REACTION_TO_SET_SIZE ) != 0; }
    bool canShrink(ReactionToSetSize reactionToSetSize) const { return !reactionToSetSize || this->canShrinkInReactionToSetSize(); }
    virtual bool isTabable() const;
    // If the tab event will be sent to this widget. When false tab simply works as focus next/previous.
    virtual bool consumesTab() const { return false; }
    virtual bool shouldClipRendering() const { return true; }
    virtual bool interactsWithStyleView() const { return true; }
    virtual TextInputInfo queryTextInputInfo();
    /** @return This widget's index in its parent's public children std::vector.
     * @return -1 if this widget has no parent or is a private child of its parent. */
    int getIndexInParent() const;
    /** @return A rectangle where the top left Point is the widget's location.
     * The size of the rectangle is this widget's size. */
    Rectangle getRelativeRectangle() const;
    /** @return A rectangle where the top left Point is 0,0. The size of the rectangle is this widget's size. */
    Rectangle getSizeRectangle() const;
    virtual const Point& getLocation() const; // @return The location relative to the parent widget content rectangle
    Rectangle getAbsoluteRectangle()  const; // A rectangle where the top left Point is the widget's absolute position.
    Rectangle getRectangleRelativeTo(const Point& relativeTo)  const; // Optimized version if absolutePosition is available
    const Dimension& getSize() const { return this->size; }
    int getWidth() const { return getSize().width; }  // not virtual - always returns the width of virtual getSize()
    int getHeight() const { return getSize().height; }  // not virtual - always returns the height of virtual getSize()
    /** The Gui calls this to know if the mouse is over this widget.
     * By default, this checks if the point is inside the widget's size rectangle.
     * Although certain widgets override this to check the widget's inner rectangle.
     * @return A boolean indicating if this relative point is inside the widget.
     * @param point Point to test relative to the parent widget's content area (excluding padding). */
    virtual bool intersectionWithPoint(const Point& point) const;
    /** @return True if the widget is visible, should be rendered, is not ignored by interaction, and if the given point
     * is contained within the widget's bounds and clipping box.
     * @param point Point to test relative to the parent widget's content area (excluding padding). */
    bool isUnderMouse(const Point& point) const;
    /** Positions this widget to the given anchor in its parent.
    * @param alignment The alignment to align this widget to. */
    virtual void alignToParent(AreaAlign alignment);
    virtual const std::string& getText() const; // @return The UTF8 encoded text string of this widget.
    virtual void setSize(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo()); // Dimension and will clamp it to the minimum and maximum sizes.
    void setSizeInternal(int width, int height);
    void setSizeForce(int width, int height, SetSizeInfo setSizeInfo = SetSizeInfo());
    virtual void focusGained(TabbedIn tabbedIn); // Called by the Gui when this widget gains input focus.
    virtual void focus(TabbedIn tabbedIn = TabbedIn::False); // Will try to give this widget input focus. Will not work if the widget is not focusable.
    virtual void clearFocus();
    virtual void focusLost(); // Called by the Gui when this widget loses input focus.
    /** Called by the Gui when this widget needs to be painted. */
    void paint(const PaintEvent& paintEvent, const Point& absolutePosition);
    virtual Widget& setEnabled(bool enabled);
    agui::Widget* getWidgetRecursively(const std::function<bool(agui::Widget*)>& predicate);
    template <class T> T* getNthWidget(int n, const std::function<bool(T*)>& predicate = nullptr)
    {
      agui::Widget* result = this->getWidgetRecursively([&predicate, &n](agui::Widget* widget)
                                                        {
                                                          if (T* casted = dynamic_cast<T*>(widget))
                                                            if (!predicate || predicate(casted))
                                                            {
                                                              --n;
                                                              if (n == 0)
                                                                return true;
                                                            }
                                                          return false;
                                                        });
      return static_cast<T*>(result);
    }
    template <class T> T* getWidget(const std::function<bool(T*)>& predicate = nullptr)
    {
      agui::Widget* result = this->getWidgetRecursively([&predicate](agui::Widget* widget)
                                                        {
                                                          if (T* casted = dynamic_cast<T*>(widget))
                                                            return !predicate || predicate(casted);
                                                          return false;
                                                        });
      return static_cast<T*>(result);
    }
    void recursiveSetEnabled(bool enabled);
    virtual void setLocation(const Point& location); // The location is relative to its parent.
    virtual void setLocation(int x, int y); // Sets the location to the parameter x and y. The location is relative to its parent.
    virtual void setText(const std::string& text);
    virtual void setText(std::string&& text);
    void setText(const char*) = delete;
    Point getAbsolutePosition() const;
    // The position of the widget within the specified ancestor widget. If the given widget is nullptr or not an
    // ancestor, then this computes the position of the widget relative to the last non-null ancestor widget (which
    // should be its absolute position).
    Point getRelativePositionTo(const Widget* targetAncestor) const;
    Rectangle getAbsoluteClippingRectangle() const;
    Rectangle getRelativeClippingRectangleTo(const Widget* targetAncestor) const;
    bool hasParent(Widget* parentToFind) const;
    Point getChildRelativePosition(const Widget* child) const; // The relative position of the given immediate child widget to the current widget.
    Rectangle getChildRelativeClippingRectangle(const Widget* child) const; // The clipping rectangle of the given immediate child widget relative to the current widget.
    void bringToFront(); // Makes this widget the front most child in its parent. This affects the render order.
    bool isOnTop() const;
    void sendToBack(); // Makes this widget the back most child in its parent. This affects the render order.
    /** This will focus this widget's next child.
     * It will focus the first child if its last child is focused or none of its children have focus. */
    virtual void focusNext();
    /** This will focus this widget's previous child.
     * It will focus the last child if its first child is focused or none of its children have focus. */
    virtual void focusPrevious();
    /** @return A boolean indicating if this widget has the input focus. */
    virtual bool isFocused() const;
    /** Called when the Gui's logic method is called and the parent is not handling it.
     * @param timeElapsed The amount of time the application has been running.
     * This method is useful for animated and timed events. */
    virtual void logic(double timeElapsed) { (void)timeElapsed; }
    virtual int getTextLength() const; // @return The number of UTF8 characters in the widget's text.
    virtual int updateTextLength();
    void setFocusable(bool focusable); // Sets whether or not this widget can receive input focus.
    void keepFocusWhenClickingOutsideOnNonFocusableWidget();
    void dontKeepFocusWhenClickingOutsideOnNonFocusableWidget();
    bool shoulKeepFocusWhenClickingOutsideOnNonFocusableWidget();
    virtual void setFocusParentWhenNotFocusable(bool value); // If the parent should be focused instead of this when not focusable
    bool getFocusParentWhenNotFocusable() const;
    virtual bool shouldSkipForFocusNext() const { return false; } // Widget and its children are focusable directly but not by focus next/previous (tabbing)
    virtual void setTabable(bool tabable); // Sets whether or not this widget can be tabbed to.

    Widget& onScaleSetup(GenericTargetable* owner, std::function<void()> callback);
    Widget& onClick(GenericTargetable* owner, std::function<void()> callback);
    Widget& onClick(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onDoubleClick(GenericTargetable* owner, std::function<void()> callback);
    void onDoubleClick(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onToggle(GenericTargetable* owner, std::function<void()> callback);
    void onToggle(GenericTargetable* owner, std::function<void(bool leftClick)> callback);
    void onConfirm(GenericTargetable* owner, std::function<void()> callback);
    void onConfirm(GenericTargetable* owner, std::function<void(const agui::KeyEvent& keyEvent)> callback);
    void onItemSelectConfirm(GenericTargetable* owner, std::function<void(int index)> callback);
    void onMouseEnter(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseLeave(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseHover(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onTextEdit(GenericTargetable* owner, std::function<void()> callback);
    void onFocusGain(GenericTargetable* owner, std::function<void()> callback);
    void onFocusLose(GenericTargetable* owner, std::function<void()> callback);
    void onItemDrag(GenericTargetable* owner, std::function<void(int fromIndex, int toIndex)> callback);
    void onMouseDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseMove(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseDrag(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseWheelDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseWheelUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseWheelLeft(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onMouseWheelRight(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onModalMouseDown(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onModalMouseUp(GenericTargetable* owner, std::function<void(const MouseEvent&)> callback);
    void onKeyDown(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback);
    void onKeyUp(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback);
    void onKeyRepeat(GenericTargetable* owner, std::function<void(const KeyEvent&)> callback);
    void onCenter(GenericTargetable* owner, std::function<void()> callback);

    virtual CursorProvider::CursorEnum getEnterCursor() const; // The cursor that should be set when the mouse enters the widget.
    bool setCursor(CursorProvider::CursorEnum cursor); // @return true if the parameter cursor was set
    virtual void resizeToContents();
    virtual void resizeToContentsRecursive();
    void checkLostFocusRecursive();
    void checkLostWidgetUnderMouseRecursive();
    /** By clearing parent widget, all of my children will lose top.
     * If the top focus manager points to them, they would have no chance to unfocus when they are destroyed,
     * that would result in the focus manager pointing to deleted widget. */
    virtual void clearParentWidget(KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False);
    void destroyIfautoDestructWhenRemovedFromParent();
    virtual int getTopPadding() const;
    virtual int getRightPadding() const;
    virtual int getBottomPadding() const;
    virtual int getLeftPadding() const;
    Point getLeftTopPadding() const;
    virtual void getPaddings(int* left, int* right, int* top, int* bottom) const;
    virtual int getTopBorder() const;
    virtual int getRightBorder() const;
    virtual int getBottomBorder() const;
    virtual int getLeftBorder() const;
    virtual const ElementImageSet* getBorderImageSet() const { return nullptr; }

    virtual int getAdditionalTopPadding() const { return 0; }
    // @return A rectangle where the top left Point is left,top margin The size of the rectangle is this widget's inner size.
    virtual Rectangle getContentRectangle() const;
    virtual Rectangle getContentRectangleWithoutPadding() const;
    virtual Dimension getContentSize() const;
    virtual Rectangle getContentSizeAsRectangle() const; // Equivalent to Rectangle(Point(0,0), getContentRectagnle().getSize())
    virtual void setContentSize(int width, int height);
    virtual void setContentSizeInternal(int width, int height);
    void setContentSize(const Dimension& size);
    int getContentWidth() const;
    int getContentHeight() const;
    int getHorizontalPaddings() const;
    int getVerticalPaddings() const;
    int getHorizontalMargins() const;
    int getVerticalMargins() const;
    virtual int getTopMargin() const;
    virtual int getBottomMargin() const;
    virtual int getLeftMargin() const;
    virtual int getRightMargin() const;
    virtual TransparentValue isTransparent() const { return TransparentValue::No; }
    virtual bool isListBox() const { return false; }
    virtual bool isToolTip() const { return false; }
    virtual bool isScrollBar() const { return false; }
    virtual Button* asButton() { return nullptr; }
    virtual const Button* asButton() const { return nullptr; }
    virtual Slider* asSlider() { return nullptr; }
    virtual const Slider* asSlider() const { return nullptr; }
    virtual ScrollPane* asScrollPane() { return nullptr; }
    virtual const ScrollPane* asScrollPane() const { return nullptr; }
    virtual TableWithSelection* asTableWithSelection() { return nullptr; }
    virtual const TableWithSelection* asTableWithSelection() const { return nullptr; }
    virtual Window* getDragTarget() { return nullptr; }
    virtual const Window* getDragTarget() const { return nullptr; }
    bool isHorizontallyStretchable() const;
    bool isVerticallyStretchable() const;
    bool isVerticallySquashable() const; // this only returns whether the the widget has the squashable property
    virtual int maximumVerticalSquashSize() const; // Either by squashing squshable or un-stretching stretchable
    virtual int maximumHorizontalSquashSize() const; // Either by squashing squshable or un-stretching stretchable
    bool isHorizontallySquashable() const;
    void displaySizeChangedRecursive();
    void postDisplaySizeChangedRecursive();
    virtual void displaySizeChanged();
    virtual void postDisplaySizeChanged();
    virtual void flagsChanged();
    //for windows that are on top of remote view(train gui, logistic gui), return the widget that gamepad should interact with.
    virtual Widget* getRemoteViewInternalVisibleWidget() { return nullptr; }
    /** Recursively searches the widget hierarchy for the last descendent that intersects the given mouse position.
    * @param mousePosition The mouse position relative to the screen's origin (0, 0)
    * @param thisAbsolutePosition The absolute position of the current widget relative to the screen's origin. It's cheaper to use this parameter (O(1)) than to call getAbsolutePosition() (O(N)).
    * @param absolutePositionClip The absolute clipping rectangle to consider. Children outside of this rectangle should be ignored.
    * @return The last visible descendant widget that contains the given mouse position within its bounds. */
    virtual Widget* getWidgetUnderMouse(Point mousePosition, Point thisAbsolutePosition, Rectangle absolutePositionClip, TransparentValue parentTransparent);
    Widget* getUnderMouseWithTransparency(TransparentValue transparent);
    Widget& squashHorizontally();
    Widget& dontSquashHorizontally();
    Widget& squashVertically();
    Widget& dontSquashVertically();
    Widget& squash();
    Widget& stretchHorizontally();
    Widget& stretchAndExpandHorizontally();
    Widget& dontStretchHorizontally();
    Widget& stretchVertically();
    Widget& stretchAndExpandVertically();
    Widget& dontStretchVertically();
    Widget& stretch();
    Widget& dontStretch();
    Widget& setLeftMargin(int16_t margin);
    Widget& setRightMargin(int16_t margin);
    Widget& setTopMargin(int16_t margin);
    Widget& setBottomMargin(int16_t margin);
    Widget& setVerticalAlign(agui::VerticalAlign alignment);
    Widget& setHorizontalAlign(agui::HorizontalAlign alignment);
    void setFlushToTopOfStyle(const agui::Style& style);
    void setFlushToBottomOfStyle(const agui::Style& style);
    Widget& setFor(Widget* other); // Used for labels connected to checkboxes, but works in a general way
    Widget& sharesTooltipWith(Widget* other);
    void callRecursively(const std::function<void(Widget*)>& callback);
    virtual ToolTip* createToolTip() { return nullptr; }
    ToolTip* getToolTip();
    const ToolTip* getToolTip() const;
    const ToolTipCreatorBase* getTooltipCreator() const { return this->toolTipCreator; }
    ToolTip* checkCreateStyleTooltip();
    void addWidgetInfoComment(VerticalFlow& result, const std::string& caption, const std::string& value) const;
    void createStyleTooltipContents(VerticalFlow& result);
    void createWidgetInfoTooltipContents(VerticalFlow& result) const;
    // Removes the tooltip widget (automatically called when the mouse leaves the widget) but leaves the tooltip creator intact.
    bool removeToolTipWidget(bool forceDelete = false);
    void updateTooltipPosition(const Point& mousePosition);
    void checkCreateTooltip();
    void setToolTipCreator(ToolTipCreatorBase* creator); // takes ownership!
    Widget& setToolTipWithInfoIcon(const std::string& text);
    Widget& setToolTipWithInfoIcon(std::string&& text);
    Widget& setToolTip(const std::string& text);
    Widget& setToolTip(std::string&& text);
    Widget& appendTooltip(std::string&& text);
    Widget& setToolTip(const std::string& title, const std::string& text);
    Widget& setToolTip(std::string&& title, std::string&& text);
    // Clears the tooltip creator and removes the tooltip widget. The same as calling setToolTip(std::string());
    void clearToolTip() { this->setToolTip(std::string()); }
    bool hasToolTipCreator() { return this->toolTipCreator != nullptr; }
    Widget& operator<<(Widget& other) { this->add(&other); return *this; }
    Widget& operator<<(Widget* other) { this->add(other); return *this; }
    template<class T> requires std::is_base_of_v<Widget, T>
    Widget& operator<<(std::unique_ptr<T>& other) { this->add(other.get()); return *this; }
    Widget& operator<<(const Empty& empty);
    Widget& operator<<(const HorizontalPusher& pusher);
    Widget& operator<<(const VerticalPusher& pusher);

    virtual bool dragOnlyByLeftMouseButton() const { return false; }
    bool isDragEnabled() const { return (this->usageBitMask & DRAG_ENABLED) != 0; }
    void setDragEnabled() { this->usageBitMask |= DRAG_ENABLED; }
    void clearDragEnabled() { this->usageBitMask &= ~DRAG_ENABLED; }
    bool isRequireMinimumDragDistance() const { return (this->usageBitMask & REQUIRE_MINIMUM_DRAG_DISTANCE) != 0; }
    void setRequireMinimumDragDistance() { this->usageBitMask |= REQUIRE_MINIMUM_DRAG_DISTANCE; }
    void clearRequireMinimumDragDistance() { this->usageBitMask &= ~REQUIRE_MINIMUM_DRAG_DISTANCE; }
    bool isCullingDrawing() const { return (this->usageBitMask & CULL_DRAWING) != 0; }
    void setCullingDrawing() { this->usageBitMask |= CULL_DRAWING; }
    void clearCullingDrawing() { this->usageBitMask &= ~CULL_DRAWING; }
    void triggerResize();
    bool canDecreaseWidth() const { return (this->usageBitMask & DONT_DECREASE_WIDTH) == 0; }
    Widget& dontDecreaseWidth();
    Widget& allowDecreaseWidth();
    Widget& dontDecreaseHeight();
    Widget& allowDecreaseHeight();
    Widget& dontDecreaseSize();
    Widget& allowDecreaseSize();
    Widget& setAllowDecreaseSize(bool allow);
    void autoDestructWhenRemovedFromParent() { this->usageBitMask |= AUTO_DESTRUCT_WHEN_REMOVED_FROM_PARENT; }
    Dimension getSizeBeforeStretching() const { return this->sizeBeforeStretching; }
    // Still takes up layout space just doesn't render (as the name suggests)
    void setRender(bool value);
    bool shouldRender() const { return (this->usageBitMask & RENDER) != 0; }
    bool isIncludedInMouseButtonFilter(MouseButton button) const;
    void fireClickOnMouseDown() { this->usageBitMask |= FIRE_CLICK_ON_MOUSE_DOWN; }
    bool isFireClickOnMouseDown() const { return (this->usageBitMask & FIRE_CLICK_ON_MOUSE_DOWN) != 0; }
    void disableFireClickOnMouseDown() { this->usageBitMask &= ~FIRE_CLICK_ON_MOUSE_DOWN; }
    void setIgnoredByInteraction(bool value);
    bool isIgnoredByInteraction() const { return (this->usageBitMask & IGNORED_BY_INTERACTION) != 0; }
    void setKeepSize(bool value);
    bool isKeepSize() const { return (this->usageBitMask & KEEP_SIZE) != 0; }
    bool isReactWhenDisabled() const { return (this->usageBitMask & REACT_WHEN_DISABLED) != 0; }
    void reactWhenDisabled() { this->usageBitMask |= REACT_WHEN_DISABLED; }
    bool isResizeToContentsExplicitly() const { return (this->usageBitMask & RESIZE_TO_CONTENTS_EXPLICITLY) != 0; }
    void setResizeToContentExplicitly() { this->usageBitMask |= RESIZE_TO_CONTENTS_EXPLICITLY; }
    void clearResizeToContentExplicitly() { this->usageBitMask &= ~RESIZE_TO_CONTENTS_EXPLICITLY; }
    void setParentHovered(bool value) { if (value) this->usageBitMask |= PARENT_HOVERED; else this->usageBitMask &= ~PARENT_HOVERED; }
    bool isParentHovered() const { return (this->usageBitMask & PARENT_HOVERED) != 0; }
    bool setShownBySearch(bool value);
    void hideBySearch();
    void showBySearch();
    bool isHiddenBySearch() const { return (this->usageBitMask & HIDDEN_BY_SEARCH) != 0; }
    virtual bool contains(const LowercaseString& filter);
    virtual bool genericSearch(const LowercaseString& filter);
    Widget& neverHideBySearch();
    bool canBeHiddenBySearch() const;
    Widget& ignoreBySearch();
    Widget& dontIgnoreBySearch();
    bool isIgnoredBySearch() const;
    void setAtomicSearch() { this->usageBitMask |= ATOMIC_SEARCH; }
    bool isAtomicSearch() const { return (this->usageBitMask & ATOMIC_SEARCH) != 0; }
    void setSkipDuplicateWidgetCheck() { this->usageBitMask |= SKIP_DUPLICATE_WIDGET_CHECK; }
    void clearSkipDuplciateWidgetCheck() { this->usageBitMask &= ~SKIP_DUPLICATE_WIDGET_CHECK; }
    void setSkipChildLogic() { this->usageBitMask |= SKIP_CHILD_LOGIC; }
    bool shouldSkipChildLogic() const { return (this->usageBitMask & SKIP_CHILD_LOGIC) != 0; }
    FocusManager* getFocusManager() const;

    virtual Clickable* asClickable() { return nullptr; }

    virtual bool isInset() const;
    static const Style& defaultWidgetStyle;

    //if this widget should interact with a Game Controller, returns a child widget(or this) that should be hovered.
    Widget* getGameControllerHoveredChild();
  protected:
    //if not nullptr it means this widget's bounding box is considered as a whole but the returned widget will be hovered.
    //this makes sense for widgets like sliders where the widget should be selected regardless of thumb position, but the hovered widget should be the thumb.
    //for simple widgets like buttons, it should return this
    virtual Widget* getGameControllerHoveredChildInternal() { return nullptr; }

  public:
#ifdef DEBUG
    static std::unordered_set<Widget*> widgets;
    bool childrenBeingIterated = false;
    bool privateChildrenBeingIterated = false;
#endif

  private:
    constexpr static uint32_t FLAGGED_FOR_DESTRUCTION = 1 << 0;
    constexpr static uint32_t SKIP_DUPLICATE_WIDGET_CHECK = 1 << 1;
    constexpr static uint32_t VISIBLE = 1 << 2;
    constexpr static uint32_t ENABLED = 1 << 3;
    constexpr static uint32_t FOCUSABLE = 1 << 4;
    constexpr static uint32_t TABABLE = 1 << 5;
    constexpr static uint32_t FOCUS_PARENT_WHEN_NOT_FOCUSABLE = 1 << 6;
    constexpr static uint32_t DONT_DECREASE_WIDTH = 1 << 7;
    constexpr static uint32_t DONT_DECREASE_HEIGHT = 1 << 8;
    constexpr static uint32_t AUTO_DESTRUCT_WHEN_REMOVED_FROM_PARENT = 1 << 9;
    constexpr static uint32_t RENDER = 1 << 10; // unlike setting style.setVisible(false), this keeps the widget in the layout
    constexpr static uint32_t FIRE_CLICK_ON_MOUSE_DOWN = 1 << 11;
    constexpr static uint32_t IGNORED_BY_INTERACTION = 1 << 12;
    constexpr static uint32_t KEEP_SIZE = 1 << 13; // Don't do the default Widget::resizeToContents logic on this object
    constexpr static uint32_t REACT_WHEN_DISABLED = 1 << 14; // reacts to clicking even when it is disabled
    constexpr static uint32_t RESIZE_TO_CONTENTS_EXPLICITLY = 1 << 15;

    // by default, the widgets keep their size when resized from parent
    // Label, HorizontalFlow and VerticalFlow has this ON by default and some special widgets have custom resizing behavior
    constexpr static uint32_t SHRINK_IN_REACTION_TO_SET_SIZE = 1 << 16;
    constexpr static uint32_t HIDDEN_BY_SEARCH = 1 << 17;
    constexpr static uint32_t KEEP_FOCUS_WHEN_CLICKING_OUTSIDE_ON_NON_FOCUSABLE_WIDGET = 1 << 18;
    constexpr static uint32_t PARENT_HOVERED = 1 << 19; // For example a label in a button or TableWithSelection being hovered
    constexpr static uint32_t ATOMIC_SEARCH = 1 << 20; // this element can be searched only as a whole

    // If enabled (default), the view will receive mouse drag events if the mouse is used to drag the view.
    constexpr static uint32_t DRAG_ENABLED = 1 << 21;
    // When enabled, drag events should wait to fire until after the mouse has moved some minimum distance after mouse down.
    // When disabled (default), drag events can begin firing immediately after the mouse moves by 1 pixel.
    // Enable this flag if a view is both clickable and draggable to better disambiguate between the two gestures.
    constexpr static uint32_t REQUIRE_MINIMUM_DRAG_DISTANCE = 1 << 22;
    constexpr static uint32_t CULL_DRAWING = 1 << 23;
    constexpr static uint32_t SKIP_CHILD_LOGIC = 1 << 24;
  public:
    static const Style& getStyleStatic();

    std::string getParentPathString() const;

  protected:
    void setGuiInstanceRecursively(Gui* instance);
  private:
    mutable Gui* guiInstance = nullptr;
  protected:
    GenericTargeter<ToolTip> toolTip;
    ToolTipCreatorBase* toolTipCreator = nullptr;

    std::vector<Listener> actionListeners;
  private:

    Widget* parentWidget = nullptr;
    WidgetArray children;
    WidgetArray privateChildren;

    uint32_t usageBitMask = VISIBLE | ENABLED | RENDER | FIRE_CLICK_ON_MOUSE_DOWN | RESIZE_TO_CONTENTS_EXPLICITLY | DRAG_ENABLED;
    Point location;
    Dimension size;
  protected:
    Dimension sizeBeforeStretching;
    /* Flow can easily contain widgets that get outside its content rectangle (negative margin for example).
    * So we need to know the actual rectangle containing this and all children of this when recursively deciding what is under mouse */
    mutable std::optional<Rectangle> clippingRectangle;
  private:
    std::string text;
    int textLen = 0;
  public:
    MouseButton mouseButtonFilter = MouseButton::ALL;
    GameControllerInteraction gameControllerInteraction = GameControllerInteraction::Normal;
  };

  enum class SingleLine { Derive, True, False };
  // creates label with AUTO_DESTRUCT_WHEN_REMOVED_FROM_PARENT
  Label& label(const std::string& text, const LabelStyle* parentStyle = nullptr, SingleLine singleLine = SingleLine::Derive);
  Label& label(const std::string& text, SingleLine singleLine);
  Label& label(std::string&& text, const LabelStyle* parentStyle = nullptr, SingleLine singleLine = SingleLine::Derive);
  Label& label(std::string&& text, SingleLine singleLine);
  Label& label(const std::string& text, RichTextSetting richTextSetting);
  Label& label(std::string&& text, RichTextSetting richTextSetting);
  Label& labelWithToolTip(const std::string& text, const std::string& toolTipText, const LabelStyle* parentStyle = nullptr, SingleLine singleLine = SingleLine::Derive);
  Label& labelWithToolTip(std::string&& text, std::string&& toolTipText, const LabelStyle* parentStyle = nullptr, SingleLine singleLine = SingleLine::Derive);
  Label& labelWithToolTipWithInfoIcon(std::string&& text, std::string&& toolTipText, const LabelStyle* parentStyle = nullptr, SingleLine singleLine = SingleLine::Derive);
  ImageWidget& image(std::unique_ptr<Image> image, const ImageStyle* parentStyle = nullptr);
  ImageWidget& image(const ImageStyle* parentStyle = nullptr);
  Table& table(int columnCount, const TableStyle* parentStyle = nullptr);
  Frame& frame(GuiDirection guiDirection, const FrameStyle* parentStyle);
  Frame& frame(const FrameStyle* parentStyle = nullptr);
  Frame& frame(std::string&& text, const FrameStyle* parentStyle);
  Tab& tab(const std::string& text, const TabStyle* parentStyle = nullptr);
  CheckBox& checkBox(bool checked, std::string&& text, Widget* callbackHolder, std::function<void(bool)> callback);
  CheckBox& checkBoxWithTooltipWithInfoIcon(bool checked, std::string&& text, std::string&& tooltip, Widget* callbackHolder, std::function<void(bool)> callback);
  Button& button(std::string&& text, Widget* callbackHolder, std::function<void()> callback, const ButtonStyle* parentStyle = nullptr);
  EmptyWidget& filler(const EmptyWidgetStyle* parentStyle = nullptr);
  VerticalLine& verticalLine(const LineStyle* parentStyle = nullptr);
  template<class WidgetType>
  WidgetType* hold(WidgetType* widget)
  {
    widget->autoDestructWhenRemovedFromParent();
    return widget;
  }
  template<class WidgetType>
  WidgetType& hold(WidgetType& widget)
  {
    widget.autoDestructWhenRemovedFromParent();
    return widget;
  }

  template<class T> requires std::is_base_of_v<Widget, T>
  bool operator==(const Widget* a, const std::unique_ptr<T>& b)
  { return a == b.get(); }
  template<class T> requires std::is_base_of_v<Widget, T>
  bool operator!=(const Widget* a, const std::unique_ptr<T>& b)
  { return a != b.get(); }
  template<class T> requires std::is_base_of_v<Widget, T>
  bool operator==(const std::unique_ptr<T>& a, const agui::Widget* b)
  { return a.get() == b; }
  template<class T> requires std::is_base_of_v<Widget, T>
  bool operator!=(const std::unique_ptr<T>& a, const Widget* b)
  { return a.get() != b; }

  class ScopedDisableDuplicateChecking
  {
  public:
    explicit ScopedDisableDuplicateChecking(Widget& widget) : widget(widget) { widget.setSkipDuplicateWidgetCheck(); }
    ScopedDisableDuplicateChecking(ScopedDisableDuplicateChecking&&) = delete;
    ScopedDisableDuplicateChecking(const ScopedDisableDuplicateChecking&) = delete;
    ~ScopedDisableDuplicateChecking() { this->widget.clearSkipDuplciateWidgetCheck(); }

    ScopedDisableDuplicateChecking& operator=(const ScopedDisableDuplicateChecking&) = delete;
    ScopedDisableDuplicateChecking& operator=(ScopedDisableDuplicateChecking&&) = delete;

    Widget& widget;
  };
}

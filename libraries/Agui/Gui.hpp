#pragma once
#include <Agui/CursorProvider.hpp>
#include <Agui/FocusManager.hpp>
#include <Agui/GenericTargeter.hpp>
#include <Agui/GuiFlyingText.hpp>
#include <Agui/KeyEvent.hpp>
#include <Agui/MouseInput.hpp>
#include <Agui/TextInputInfo.hpp>
#include <Agui/Transform.hpp>
#include <Agui/Widget/TextField.hpp>
#include <Agui/Widget/ToolTip.hpp>
#include <queue>
#include <stack>
#include <vector>
#include <memory>

namespace agui
{
  class CursorProvider;
  class Graphics;
  class KeyboardInput;
  class Widget;
  class Input;
}

namespace agui
{
  class TopContainer;
  struct ImeCompositionInfo;
  /**
   * @mainpage
   * @section Welcome
   * This is the documentation for the classes in the Agui Library.
   *
   * Agui is a cross platform and back end independent library for Graphical User Interfaces in games.
   * This means that any code you write with
   * it will work in whatever environment you are working with. At this time, only an Allegro 5
   * back end is developed, but you are free to develop your own.
   *
   * In addition, every class in Agui that uses text uses UTF8.
   * This means that Unicode is natively supported throughout.
   */

  /**
   * You should create one of these for every Gui you need.
   * All desktop Widgets must be added through this.
   * This class requires you to setInput and setGraphics for it to work correctly. */
  class Gui
  {
    friend class Widget;
    /** Converts the mouse event's position into one that is relative to the parameter widget. */
    MouseEvent convertMouseEventToRelative(Widget* source, const MouseEvent& mouseEvent);
    void handleHover();
    void handleDoubleClick();
    void resetDoubleClickTime();
    /** Invalidates the hover event. */
  public:
    void resetHoverTime();
    void resetGuiTooltipHoverTime();
    void resetEntityTooltipHoverTime();
    void clearTooltips();

  private:
    void setKeyEvent(const KeyboardInput& keyboard, bool handled);
    void setLastMouseDownControl(Widget* control);
    MouseEvent createMouseEvent(const MouseInput& mouse);
    void setMouseButtonDown(MouseButton button); // Sets which mouse button is pressed.
  public:
    Widget* recursiveGetWidgetUnderMouse(Point position); // Finds which widget is under the mouse using recursion.
  private:
    void handleTimedEvents();

    const Widget* getLastMouseDownControl() const;
    Widget* getLastMouseDownControl();
    MouseButton getMouseButtonDown() const;
    bool widgetIsModalChild(Widget* widget) const;

    bool recursiveFocusNext(Widget* target, Widget* focused); // @return True if the focused widget was passed.
    virtual void focusNextTabableWidget();
    bool recursiveFocusPrevious(Widget* target, Widget* focused); // @return @c true if the focused widget was passed.
    virtual void focusPreviousTabableWidget();

    // events
    void handleMouseDown(const MouseInput& mouse);
    void handleMouseUp(const MouseInput& mouse);
    void handleKeyDown(const KeyboardInput& keyboard);
    void handleKeyUp(const KeyboardInput& keyboard);
    void handleKeyRepeat(const KeyboardInput& keyboard); // Handles a key being held and not released thereby triggering a repeat.
    void recursiveDoLogic(Widget* baseWidget); // Calls Widget::logic() for every widget starting at base widget.
    void clearInvalidTooltips();
    /** Removes the widget from the Gui. It essentially nullptrs all pointers of the parameter widget used by the Gui
     * to avoid crashes if a widget was under the mouse at the time of its death. */
    void removeWidget(Widget* widget);
    bool handleTabbing();
    void dispatchKeyboardEvents(); // Dispatches the queued keyboard events.
    void dispatchMouseEvents(); // Dispatches the queued mouse events.
    void orderToUpdateWidgetUnderMouse();
    void updateFlyingTexts();
    void updateTextInputInfo();
    void updateImeComposition();

  public:
    void handleMouseAxes(const MouseInput& mouse); // Handles mouse move and mouse wheel events.
    void handleMouseAxes(MouseEvent mouseEvent);
    void handleMouseDown(const MouseEvent& mouseEvent);
    void handleMouseUp(MouseEvent mouseEvent);
    void handleKeyDown(const KeyEvent& keyEvent);
    void handleKeyUp(const KeyEvent& keyEvent);
    void addFlyingText(Point position, const std::string& text, Color color, const Font* font, int timeToLive, int screnWidth);
    Widget* getFocusedWidget(); // @return The focused widget or nullptr if none are focused.
    Widget* getWidgetOnTop(); // @return The back most widget (the one on top of everything else)
    Widget* getNonTooltipWidgetOnTop();
    bool somethingHasModalFocus() const { return this->focusManager.somethingHasModalFocus(); }
    void flagForForceHover(); // Flags this Gui to force hover at the end of the current or next update cycle.
    bool flaggedForForceHover() const; // True if this Gui is flagged for force hover.
    void forceHover(); // Calls hover even without waiting for hover interval.
    void clearFocus(); // No widget will be focused after this.
    void modalChanged(); // Releases widget under mouse

    void callAfterResizeIsSattledAction(Widget* widget);
    void addAnchoredWidget(Window& window);
    ImeCompositionInfo getImeCompositionInfo();

    virtual Widget* getWidgetUnderMouse();
    void setWidgetUnderMouse(Widget* widget); // for testing
    void clearWidgetUnderMouse();
    void dispatchWidgetDestroyed(Widget* widget); // Calls _removeWidget.
    void widgetLocationChanged(); // Called by a widget when its location, size, or visibility changes.
    void setTabbingEnabled(bool tabbing);
    bool isTabbingEnabled() const;
    /** Default is KEY_TAB. */
    virtual void setTabNextKey(KeyEnum key,
                               ExtendedKeyEnum extKey = EXT_KEY_NONE,
                               bool shift = false,
                               bool control = false,
                               bool alt = false);
    /** Default is KEY_TAB + control. */
    virtual void setTabPreviousKey(KeyEnum key,
                                   ExtendedKeyEnum extKey = EXT_KEY_NONE,
                                   bool shift = false,
                                   bool control = false,
                                   bool alt = false);
    void setGuiTooltipHoverInterval(double time);
    void setEntityTooltipHoverInterval(double time);
    double getGuiHoverInterval() const;
    double getEntityHoverInterval() const;
    void setToolTipOffset(int32_t toolTipOffset) { this->toolTipOffset = toolTipOffset; }
    int32_t getToolTipOffset() const { return this->toolTipOffset; }
    double getDoubleClickInterval() const;
    void setDoubleClickInterval(double time);
    /** Resizes the Top widget to the size of the display. Needs the graphics context to be set. */
    void resizeToDisplay();
    /** Adds the parameter widget to the desktop if it has no parent. */
    void add(Widget* widget);
    void insert(Widget* widget, uint32_t index);
    /** Removes the parameter widget from the desktop if it is on the desktop. */
    void remove(Widget* widget);
    /** Should be called every time your game loop updates.
     * It will poll the Input, dequeue all queued mouse and keyboard events, and
     * call Widget::logic on every widget in the Gui.*/
    virtual void logic(bool regularUpdate);
    /** @return The amount of time in seconds the application has been running. Useful for timed events. */
    double getElapsedTime() const;
    Gui();
    /** Will paint every widget in the Gui and their children.
     * Call this each time you render. */
    void render();
    /** Set the graphics context for the Gui. Will resize the Gui to the display size. */
    void setGraphics(Graphics* context);
    /** Set the input for the Gui. Will resize the Gui to the display size. */
    void setInput(Input* input);
    /** Set the size of the desktop. Call this when your display resizes. */
    void setSize(int width, int height);
    /** @return The top most, desktop widget of the Gui. Every widget in the Gui is a child of this. */
    TopContainer* getTop() const;
    /** Set whether or not widget exist checks will be made
     * when a mouse event occurs. Disable for speedup. */
    void setExistanceCheck(bool check);
    /** @return True if widget exist checks will be made
    * when a mouse event occurs. Disable for speedup. */
    int isDoingExistanceCheck() const;
    /** @Set the backend specific cursor provider. */
    void setCursorProvider(CursorProvider* provider);
    /** @Set the transform to use on the mouse.
     * @See setUseTransform */
    void setTransform(const Transform& transform);
    /** @return the transform to use on the mouse.
     * @see setUseTransform */
    const Transform& getTransform() const;
    void setUseTransform(bool use); // @Set whether or not to use a transformation on the mouse coordinates.
    bool isUsingTransform() const; // If the transform is used on the mouse.
    static void setScaleAndDisplayDensity(double scale, double displayDensity);
    bool setCursor(CursorProvider::CursorEnum cursor); // If the cursor provider is set and it successfully set the cursor
    void forceReleaseControlWithLock();
    Widget* getLockWidget(); // @return The widget with drag focus or nullptr
    void requestBringWidgetToFront(Widget* widget);
    Graphics* getGraphicsContext() { return this->graphicsContext; }
    void destroyFlaggedWidgets();
    Point getCurrentMousePosition() const { return this->currentMousePosition; }
    /** Tooltips are added with delay to avoid vector invalidation in logic loops. */
    void addToolTip(ToolTip* toolTip);
    virtual ~Gui();
    bool shouldHoverGui() const;
    bool shouldHoverEntity() const;
    void checkBringClickedWindowToFront();
    void setInstantTooltip(bool instantTooltip);
    //returns true of the screen height is below a threshold. Used to change layouts or omit some information on small screens.
    bool isScreenHeightSmall(int32_t heightThreshold = 950);
    bool isScreenWidthSmall(int32_t widthThreshold);
    void startDragging(Widget* widget);
    void resetState();
    void anchorAnchoredWidgets();

    static constexpr int wheelScrollRate = 50;
    static inline bool shouldRender = true;
    FocusManager focusManager;
    double currentTime = 0;
    Input* input = nullptr;
    Graphics* graphicsContext = nullptr;
    TopContainer* baseWidget;
    double lastTickTime = 0;
    std::queue<MouseInput> queuedMouseDown;
    MouseInput emptyMouse;
    CursorProvider* cursorProvider = nullptr;

    KeyEnum tabNextKey = KEY_TAB;
    ExtendedKeyEnum tabNextExtKey = EXT_KEY_NONE;
    bool tabNextShift = false;
    bool tabNextControl = false;
    bool tabNextAlt = false;
    KeyEnum tabPreviousKey = KEY_TAB;
    ExtendedKeyEnum tabPreviousExtKey = EXT_KEY_NONE;
    bool tabPreviousShift = true;
    bool tabPreviousControl = false;
    bool tabPreviousAlt = false;

    // used to focus a tabable widget
    bool passedFocus = false;

    bool tabbingEnabled = true;
    bool bringWindowToTopOnClick = true;

    // modal variable

    double doubleClickExpireTime = 0;
    double doubleClickInterval = 0.2;
    GenericTargeter<Widget> lastMouseDownControl;
    GenericTargeter<Widget> previousWidgetUnderMouse;
    GenericTargeter<Widget> widgetUnderMouse;
  private:
    GenericTargeter<Widget> controlWithLock; // use setters and getters because there are side-effects.
  public:
    GenericTargeter<Widget> lastHoveredControl;
    GenericTargeter<Widget> mouseUpControl;
    bool canDoubleClick = false;
    GenericTargeter<Widget> widgetOfLastClick;
    GenericTargeter<Widget> widgetToBringToFront;
    int listBoxItemIndexOfLastClick = 0;

  private:
    int dragDistanceThresholdPts = 5; // display points to move mouse before committing to a drag gesture. must be scaled by display density before use.
    int getDragDistanceThresholdPx() const { return int(this->dragDistanceThresholdPts * Gui::displayDensity); }
    bool commitToMouseDrag = false;
    Point mousePositionAtStartOfDrag;
    Point draggedWidgetShiftOnMouseDown; // used to prevent mouse drift when dragging past the threshold

  public:
    double guiTooltipHoverInterval = 2.5;
    double entityTooltipHoverInterval = 2.5;
    int32_t toolTipOffset = 0;
    double timeUntilNextGuiHover = 0;
    double timeUntilNextEntityHover = 0;
    bool wantsForceHover = false;

    KeyEvent keyEvent;
    MouseButton lastMouseButton = MouseButton::NONE;
    Point currentMousePosition;
    //always true when input method is controller
    bool mouseInWindow = true;
    bool isSimulation = false;

    /** To avoid multiple counting of widget under mouse during widget constructions, this
     * is set to true, and the update of widget under mouse is done at the end of the logic update. */
    bool updateWidgetUnderMouse = false;
    bool enableExistanceCheck = true;

    bool useTransform = false;
    Transform transform;
    std::vector<GenericTargeter<ToolTip>> tooltips;
    GenericTargeter<ToolTip> tooltipToBeAddedAtTheEndOfLogic;
    GenericTargeter<ToolTip> currentTooltip;
    GenericTargeter<ToolTip> currentStyleTooltip;
    std::vector<GenericTargeter<Widget>> callAfterResizeIsSettledActions;
    std::vector<GenericTargeter<Window>> anchoredWidgets;
    std::vector<GuiFlyingText> flyingTexts;
    GenericTargeter<TextField> imeCompositionTextField;
    TextInputInfo textInputInfo;
    bool instantTooltip = false;
    std::string infoString;
    std::string qualityString;
    std::string restartRequiredString;
    bool styleView = false;
    const LabelStyle* definingStyleLabelStyle = nullptr;
    const LabelStyle* overwritesTheSameStyleLabelStyle = nullptr;
    const LabelStyle* overwritettenStyleLabelStyle = nullptr;
    const LabelStyle* styleNameLabelStyle = nullptr;
    const TextBoxStyle* imeCompositionTextFieldStyle = nullptr;
    static double scale;
    static double displayDensity;
    static Gui* instance;
    static inline void (*log)(const std::string&) = nullptr;
  };
}

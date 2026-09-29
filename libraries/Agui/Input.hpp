#pragma once
#include "Agui/MouseInput.hpp"
#include "Agui/KeyboardInput.hpp"
#include <queue>

namespace agui
{
  struct ImeCompositionInfo;
  struct TextInputInfo;

  /** Should implement:
   *
   * A method to receive a back end specific event and convert it
   * to MouseInput or KeyboardInput.
   *
   * getTime (default uses std::clock)
   *
   * Should respect:
   *
   * isMouseEnabled
   * isKeyboardEnabled */
  class Input
  {
  public:
    enum class PlayerInputMethod: uint8_t
    {
      KeyboardAndMouse,
      GameController
    };
  protected:
    Input();
  public:
    virtual ~Input();
    /** Called by the Gui in its logic loop. Used for non event driven back ends. */
    virtual void pollInput();
    /** Pushes a mouse event which will be dequeued and processed in the next logic loop. */
    void pushMouseEvent(const MouseInput& input);
    /** Pushes a keyboard event which will be dequeued and processed in the next logic loop. */
    void pushKeyboardEvent(const KeyboardInput& input);
    bool isMouseQueueEmpty() const;
    bool isKeyboardQueueEmpty() const;
    /** Called by the Gui to process the event.
     * @return The keyboard event information and removes it from the queue. */
    KeyboardInput dequeueKeyboardInput();
    /** Called by the Gui to process the event.
     * @return The mouse event information and removes it from the queue. */
    MouseInput dequeueMouseInput();
    /** @return The amount of time the application has been running in seconds. */
    virtual double getTime() const;
    void setKeyboardEnabled(bool enabled);
    void setMouseEnabled(bool enabled);
    bool isMouseEnabled() const;
    bool isKeyboardEnabled() const;
    virtual ImeCompositionInfo getImeCompositionInfo() const;
    virtual void updateTextInput(const TextInputInfo&) {};
    virtual PlayerInputMethod getInputMethod() = 0;
    virtual void resetState();

  private:
    double startTime;
    std::queue<MouseInput> mouseEvents;
    std::queue<KeyboardInput> keyboardEvents;
    bool mouseEnabled = true;
    bool keyboardEnabled = true;
  };
}

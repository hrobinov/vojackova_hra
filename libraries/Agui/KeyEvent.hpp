#pragma once
#include "Agui/ExtendedKeyEnum.hpp"
#include "Agui/Point.hpp"
#include "Agui/MouseButton.hpp"
#include "Agui/KeyEnum.hpp"
#include <cstdint>

namespace agui { class Widget; }

namespace agui
{
  /** Raw key identifiers from the back end, passed through untouched (raylib's KeyboardKey for ours). */
  using BackendKeycode = uint32_t;
  using BackendScancode = uint32_t;

  class KeyEvent
  {
  public:
    enum KeyboardEventEnum
    {
      KEY_DOWN,
      KEY_UP,
      KEY_REPEAT
    };
    KeyEvent() = default;
    KeyEvent(Widget* source, KeyEnum key);
    KeyEvent(Widget* source, ExtendedKeyEnum key);
    /** The unichar is a UTF32 code point.*/
    KeyEvent(KeyEnum key,
             ExtendedKeyEnum extKey,
             BackendKeycode keyCode,
             BackendScancode scanCode,
             uint32_t unichar,
             double timeStamp,
             bool isAlt,
             bool isControl,
             bool isShift,
             bool isMeta,
             Widget* source = nullptr,
             bool handled = false);
    bool alt() const; /**< @return True if alt was pressed when the event occurred. */
    bool control() const; /**< @return True if control was pressed when the event occurred. */
    bool shift() const; /**< @return True if shift was pressed when the event occurred. */
    bool meta() const; /**< @return True if meta was pressed when the event occurred. */
    bool metaOrControl() const; /**< @return True if meta (on macOS) or control (elsewhere) was pressed. */
    bool optionOrControl() const; /**< @return True if option (on macOS) or control (elsewhere) was pressed. */
    /** Consumes the event. When an event is consumed, it allows the listeners to make decisions based on this. */
    void consume();
    bool isConsumed() const;
    /** @return The number of bytes this character occupies (from 1 to 4 bytes). */
    size_t getUtf8Length() const;
    /** @return The character as a std::string since UTF8 characters can be more than 1 byte. */
    std::string getUtf8String() const;
    /** @return How much time the application had been running when the event occurred.*/
    double getTimeStamp() const;
    /** @return The key code specific to the back end. */
    BackendKeycode getBackendKeycode() const;
    /** @return The scancode specific to the back end. */
    BackendScancode getBackendScancode() const;
    /** @return The UTF32 code point for this key event. */
    uint32_t getUnichar() const;
    /** @return The modifier flags specific to the back end. */
    ExtendedKeyEnum getExtendedKey() const; // @return The extended key pressed or EXT_KEY_NONE if no extended key was pressed.
    KeyEnum getKey() const; // @return The ascii key pressed or KEY_NONE if no ascii key was pressed.
    Widget* getSourceWidget() const { return this->source; }
  private:
    uint32_t unichar = 0;
    double timeStamp = 0;
    BackendKeycode keyCode = 0;
    BackendScancode scanCode = 0;
    ExtendedKeyEnum extKey;
    KeyEnum key = KEY_NONE;
    bool isAlt = false;
    bool isControl = false;
    bool isShift = false;
    bool isMeta = false;
    bool handled = false;

    Widget* source;
  };
}

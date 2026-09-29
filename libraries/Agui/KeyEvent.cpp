#include "Agui/KeyEvent.hpp"
#include "Agui/UTF8.hpp"

namespace agui
{
  KeyEvent::KeyEvent(Widget* source, KeyEnum key)
    : unichar(key)
    , extKey(ExtendedKeyEnum::EXT_KEY_NONE)
    , key(key)
    , source(source)
  {}

  KeyEvent::KeyEvent(Widget* source, ExtendedKeyEnum key)
    : extKey(key)
    , source(source)
  {}

  KeyEvent::KeyEvent(KeyEnum key, ExtendedKeyEnum extKey,
                     BackendKeycode keyCode,
                     BackendScancode scanCode,
                     uint32_t unichar,
                     double timeStamp,
                     bool isAlt, bool isControl,
                     bool isShift, bool isMeta,
                     Widget* source /*= 0*/,
                     bool handled /*= false*/)
    : unichar(unichar)
    , timeStamp(timeStamp)
    , keyCode(keyCode)
    , scanCode(scanCode)
    , extKey(extKey)
    , key(key)
    , isAlt(isAlt)
    , isControl(isControl)
    , isShift(isShift)
    , isMeta(isMeta)
    , handled(handled)
    , source(source)
  {}

  bool KeyEvent::control() const
  {
    return this->isControl;
  }

  bool KeyEvent::shift() const
  {
    return this->isShift;
  }

  BackendKeycode KeyEvent::getBackendKeycode() const
  {
    return this->keyCode;
  }

  BackendScancode KeyEvent::getBackendScancode() const
  {
    return this->scanCode;
  }

  void KeyEvent::consume()
  {
    this->handled = true;
  }

  bool KeyEvent::isConsumed() const
  {
    return this->handled;
  }

  double KeyEvent::getTimeStamp() const
  {
    return this->timeStamp;
  }

  size_t KeyEvent::getUtf8Length() const
  {
    return UTF8::getUnicharLength(this->unichar);
  }

  std::string KeyEvent::getUtf8String() const
  {
    char b[5];
    size_t sz = UTF8::encodeUtf8(b, this->unichar);
    b[sz] = 0;
    return std::string(b);
  }

  uint32_t KeyEvent::getUnichar() const
  {
    return this->unichar;
  }

  ExtendedKeyEnum KeyEvent::getExtendedKey() const
  {
    return this->extKey;
  }

  KeyEnum KeyEvent::getKey() const
  {
    return this->key;
  }

  bool KeyEvent::meta() const
  {
    return this->isMeta;
  }

  bool KeyEvent::metaOrControl() const
  {
#ifdef __APPLE__
    return this->isMeta;
#else
    return this->isControl;
#endif
  }

  bool KeyEvent::optionOrControl() const
  {
#ifdef __APPLE__
    return this->isAlt;
#else
    return this->isControl;
#endif
  }

  bool KeyEvent::alt() const
  {
    return this->isAlt;
  }
}

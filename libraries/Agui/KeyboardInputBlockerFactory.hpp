#pragma once
#include <Agui/KeyboardInputBlockType.hpp>

namespace agui
{
  class KeyboardInputBlockerBase
  {
  public:
    explicit KeyboardInputBlockerBase(KeyboardInputBlockType blockType = KeyboardInputBlockType::AlphaNumerical)
      : blockType(blockType)
    {}
    virtual ~KeyboardInputBlockerBase() = default;
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    const KeyboardInputBlockType blockType;
  };

  class KeyboardInputBlockerFactory
  {
    static KeyboardInputBlockerFactory* factory;
  public:
    virtual ~KeyboardInputBlockerFactory() {}

    virtual KeyboardInputBlockerBase* create(KeyboardInputBlockType) = 0;
    virtual KeyboardInputBlockerBase* create() = 0;

    static void setFactory(KeyboardInputBlockerFactory* factory);
    static KeyboardInputBlockerBase* createKeyboardInputBlocker(KeyboardInputBlockType);
    static KeyboardInputBlockerBase* createKeyboardInputBlocker();
  };
}

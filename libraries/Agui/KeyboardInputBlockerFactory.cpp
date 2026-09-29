#include <Agui/KeyboardInputBlockerFactory.hpp>

namespace agui
{
  KeyboardInputBlockerFactory* KeyboardInputBlockerFactory::factory = nullptr;

  void KeyboardInputBlockerFactory::setFactory(KeyboardInputBlockerFactory* factory)
  {
    delete KeyboardInputBlockerFactory::factory;
    KeyboardInputBlockerFactory::factory = factory;
  }

  KeyboardInputBlockerBase* KeyboardInputBlockerFactory::createKeyboardInputBlocker(KeyboardInputBlockType blockType)
  {
    if (!KeyboardInputBlockerFactory::factory)
      return nullptr;

    return KeyboardInputBlockerFactory::factory->create(blockType);
  }

  KeyboardInputBlockerBase* KeyboardInputBlockerFactory::createKeyboardInputBlocker()
  {
    if (!KeyboardInputBlockerFactory::factory)
      return nullptr;

    return KeyboardInputBlockerFactory::factory->create();
  }
}

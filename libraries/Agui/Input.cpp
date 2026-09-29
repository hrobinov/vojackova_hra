#include <Agui/Exception.hpp>
#include <Agui/ImeCompositionInfo.hpp>
#include <Agui/Input.hpp>
#include <ctime>

namespace agui
{
  Input::Input()
    : startTime(std::clock() / CLOCKS_PER_SEC)
  {}

  Input::~Input() = default;

  double Input::getTime() const
  {
    return (std::clock() / CLOCKS_PER_SEC) - startTime;
  }

  void Input::pushMouseEvent(const MouseInput& input)
  {
    this->mouseEvents.push(input);
  }

  void Input::pushKeyboardEvent(const KeyboardInput& input)
  {
    this->keyboardEvents.push(input);
  }

  bool Input::isMouseQueueEmpty() const
  {
    return this->mouseEvents.empty();
  }

  bool Input::isKeyboardQueueEmpty() const
  {
    return this->keyboardEvents.empty();
  }

  KeyboardInput Input::dequeueKeyboardInput()
  {
    if (isKeyboardQueueEmpty())
      throw Exception("Keyboard queue is empty!");

    KeyboardInput currentKeyInput = this->keyboardEvents.front();
    this->keyboardEvents.pop();
    return currentKeyInput;
  }

  MouseInput Input::dequeueMouseInput()
  {
    if (this->isMouseQueueEmpty())
      throw Exception("Mouse queue is empty!");

    MouseInput currentMouseInput = this->mouseEvents.front();
    this->mouseEvents.pop();
    return currentMouseInput;
  }

  void Input::setKeyboardEnabled(bool enabled)
  {
    this->keyboardEnabled = enabled;
  }

  void Input::setMouseEnabled(bool enabled)
  {
    this->mouseEnabled = enabled;
  }

  bool Input::isMouseEnabled() const
  {
    return this->mouseEnabled;
  }

  bool Input::isKeyboardEnabled() const
  {
    return this->keyboardEnabled;
  }

  ImeCompositionInfo Input::getImeCompositionInfo() const
  {
    return ImeCompositionInfo();
  }

  void Input::resetState()
  {
    while (!this->keyboardEvents.empty())
      this->keyboardEvents.pop();
    while (!this->mouseEvents.empty())
      this->mouseEvents.pop();
  }

  void Input::pollInput()
  {}
}

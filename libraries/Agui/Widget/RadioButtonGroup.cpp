#include <Agui/Widget/RadioButton.hpp>
#include <Agui/Widget/Button.hpp>
#include <Agui/Widget/RadioButtonGroup.hpp>

namespace agui
{
  template <class Type>
  Type& GenericButtonGroup<Type>::add(Type* button)
  {
    GenericButtonGroup::init(button);
    if (this->getSelected())
      GenericButtonGroup::setSelected(button, false);

    this->buttons.emplace_back(button);
    if constexpr (std::is_same_v<Type, RadioButton>)
      button->onCheck(this, [this, button]{ this->disableAllBut(button); });
    else
      button->onClick(this, [this, button]{ this->disableAllBut(button); });
    return *button;
  }

  template<class Type>
  void agui::GenericButtonGroup<Type>::setOnlySelected(Type* button)
  {
    if (button)
      GenericButtonGroup::setSelected(button, true);
    this->disableAllBut(button);
  }

  template <class Type>
  void agui::GenericButtonGroup<Type>::disableAllBut(Type* button)
  {
    for (GenericTargeter<Type>& other : this->buttons)
      if (other)
        GenericButtonGroup::setSelected(*other, other == button);
  }

  template <class Type>
  Type& GenericButtonGroup<Type>::add(Type* button, bool initialState)
  {
    GenericButtonGroup::setSelected(button, initialState);
    return this->add(button);
  }

  template <class Type>
  Type& GenericButtonGroup<Type>::hold(Type* button)
  {
    return this->add(agui::hold(button));
  }

  template <class Type>
  Type& agui::GenericButtonGroup<Type>::hold(Type* button, bool initialState)
  {
    GenericButtonGroup::setSelected(button, initialState);
    return this->hold(button);
  }

  template <class Type>
  Type* GenericButtonGroup<Type>::getSelected() const
  {
    for (GenericTargeter<Type> button : this->buttons)
      if (button && GenericButtonGroup::isSelected(*button))
        return *button;
    return nullptr;
  }

  template <class Type>
  void GenericButtonGroup<Type>::clear()
  {
    this->buttons.clear();
  }

  template <> void GenericButtonGroup<RadioButton>::setSelected(RadioButton* button, bool value) { button->setChecked(value); }
  template <> void GenericButtonGroup<Button>::setSelected(Button* button, bool value)
  {
    if (value)
      button->startToggle(true);
    else
      button->setToggleState(false);
  }

  template <> bool GenericButtonGroup<RadioButton>::isSelected(RadioButton* button) { return button->isChecked(); }
  template <> bool GenericButtonGroup<Button>::isSelected(Button* button) { return button->isToggled(); }

  template <class Type> void agui::GenericButtonGroup<Type>::init(Type*) {}
  template <> void agui::GenericButtonGroup<Button>::init(Button* button) { button->startToggle(button->isToggled()); }

  template class GenericButtonGroup<RadioButton>;
  template class GenericButtonGroup<Button>;
}

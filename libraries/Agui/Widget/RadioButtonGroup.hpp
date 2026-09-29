#pragma once
#include <Agui/GenericTargeter.hpp>
#include <vector>

namespace agui
{
  /** Groups RadioButtons/Buttons and manages them and ensure only 1 is selected. */
  template <class Type>
  class GenericButtonGroup : public GenericTargetable
  {
  public:
    Type& add(Type* button);
    Type& add(Type* button, bool initialState);
    Type& hold(Type* button);
    Type& hold(Type* button, bool initialState);
    Type* getSelected() const;
    void clear();
    void setOnlySelected(Type* button);

    size_t size() const { return this->buttons.size(); }
    Type* operator[](uint32_t index) { return *this->buttons[index]; }

    static bool isSelected(Type* button);
    static void setSelected(Type* button, bool value);
    static void init(Type* button);

  private:
    void disableAllBut(Type* button);
    std::vector<GenericTargeter<Type>> buttons;
  };

  using RadioButtonGroup = GenericButtonGroup<class RadioButton>;
  using ButtonGroup = GenericButtonGroup<class Button>;
}

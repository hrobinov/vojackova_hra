#pragma once
#include <Agui/GenericTargeter.hpp>
#include <Agui/GenericTargetable.hpp>
#include <functional>
#include <cstdint>

namespace agui
{
  class MouseEvent;
  class Widget;
  class KeyEvent;
  class Tab;
  class Window;
  class ListBox;
  struct TableIndex;

  class Listener
  {
  public:
    enum class Type
    {
      OnGuiScaleSetup,
      OnMouseClick,
      OnMouseDoubleClick,
      OnToggle,
      OnConfirm,
      OnItemSelect,
      OnItemSelectConfirm,
      OnItemDoubleClick,
      OnTableOrderingColumnChange,
      OnTableSelectionChange,
      OnTableHoverChange,
      OnTableSelectionDoubleClick,
      OnCheck,
      OnCheckChange,
      OnSwitchToggle,
      OnNumberInputValueChange,
      OnSliderMove, // used also for scrollbar
      OnMouseEnter,
      OnMouseLeave,
      OnMouseHover,
      OnTextEdit,
      OnFocusGain,
      OnFocusLose,
      OnItemDrag,
      OnMouseDown,
      OnMouseUp,
      OnMouseMove,
      OnMouseDrag,
      OnMouseWheelDown,
      OnMouseWheelUp,
      OnMouseWheelLeft,
      OnMouseWheelRight,
      OnModalMouseDown,
      OnModalMouseUp,
      OnKeyDown,
      OnKeyUp,
      OnKeyRepeat,
      SelectedTabChanged,
      OnCentered,
      OnBeforeDropdownIsShown,
      None
    };

    Listener(GenericTargetable* owner, Type type);
    Listener(const Listener& other);
    Listener(Listener&& other) noexcept;
    Listener& operator=(const Listener& other);
    Listener& operator=(Listener&& other) noexcept;
    Listener() : type(Type::None) {}
    ~Listener();
  private:
    void initFrom(const Listener& other);
    void initFrom(Listener&& other);
    void initValue();
    void destroyValue();
  public:

    GenericTargeter<GenericTargetable> owner;
    Type type;
    union
    {
      std::function<void()> onGuiScaleSetup;
      std::function<void(const MouseEvent&)> onMouseClick;
      std::function<void(const MouseEvent&)> onMouseDoubleClick;
      std::function<void(bool leftClick)> onToggle;
      std::function<void(const KeyEvent&)> onConfirm;
      std::function<void(int index, bool leftClick)> onItemSelect;
      std::function<void(int index)> onItemSelectConfirm;
      std::function<void(int index)> onItemDoubleClick;
      std::function<void(TableIndex columnIndex, bool ascending)> onTableOrderingColumnChange;
      std::function<void(TableIndex newIndex, TableIndex oldIndex, int32_t columnIndex)> onTableSelectionChange;
      std::function<void(TableIndex newIndex, TableIndex oldIndex)> onTableHoverChange;
      std::function<void(TableIndex index)> onTableSelectionDoubleClick;
      std::function<void()> onCheck;
      std::function<void(bool)> onCheckChange;
      std::function<void()> onSwitchToggle;
      std::function<void()> onNumberInputValueChange;
      std::function<void(double)> onSliderMove; // used also for scrollbar
      std::function<void(const MouseEvent&)> onMouseEnter;
      std::function<void(const MouseEvent&)> onMouseLeave;
      std::function<void(const MouseEvent&)> onMouseHover;
      std::function<void()> onTextEdit;
      std::function<void()> onFocusGain;
      std::function<void()> onFocusLose;
      std::function<void(int fromIndex, int toIndex)> itemDrag;
      std::function<void(const MouseEvent&)> onMouseDown;
      std::function<void(const MouseEvent&)> onMouseUp;
      std::function<void(const MouseEvent&)> onMouseMove;
      std::function<void(const MouseEvent&)> onMouseDrag;
      std::function<void(const MouseEvent&)> onMouseWheelDown;
      std::function<void(const MouseEvent&)> onMouseWheelUp;
      std::function<void(const MouseEvent&)> onMouseWheelLeft;
      std::function<void(const MouseEvent&)> onMouseWheelRight;
      std::function<void(const MouseEvent&)> onModalMouseDown;
      std::function<void(const MouseEvent&)> onModalMouseUp;
      std::function<void(const KeyEvent&)> onKeyDown;
      std::function<void(const KeyEvent&)> onKeyUp;
      std::function<void(const KeyEvent&)> onKeyRepeat;
      std::function<void()> onSelectedTabChange;
      std::function<void()> onCenter;
      std::function<bool()> onBeforeDropdownIsShown;
    };
  };
}

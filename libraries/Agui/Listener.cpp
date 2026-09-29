#include <Agui/Listener.hpp>
#include <Agui/Util.hpp>

namespace agui
{
  agui::Listener& Listener::operator=(const Listener& other)
  {
    this->destroyValue();
    this->type = other.type;
    this->initFrom(other);
    return *this;
  }

  agui::Listener& Listener::operator=(Listener&& other) noexcept
  {
    this->destroyValue();
    this->type = other.type;
    this->initFrom(std::move(other));
    return *this;
  }

  Listener::Listener(const Listener& other)
    : owner(other.owner)
    , type(other.type)
  {
    this->initFrom(other);
  }

  Listener::Listener(Listener&& other) noexcept
    : owner(other.owner)
    , type(other.type)
  {
    this->initFrom(std::move(other));
  }

  Listener::Listener(GenericTargetable* owner, Type type)
    : owner(owner)
    , type(type)
  {
    this->initValue();
  }

  Listener::~Listener()
  {
    this->destroyValue();
  }

  void Listener::initFrom(const Listener& other)
  {
#define HANDLE_CASE(CASE, MEMBER) \
  case CASE: new (&this->MEMBER) decltype(this->MEMBER)(other.MEMBER); break
    switch (this->type)
    {
      HANDLE_CASE(Type::OnGuiScaleSetup, onGuiScaleSetup);
      HANDLE_CASE(Type::OnMouseClick, onMouseClick);
      HANDLE_CASE(Type::OnMouseDoubleClick, onMouseDoubleClick);
      HANDLE_CASE(Type::OnToggle, onToggle);
      HANDLE_CASE(Type::OnConfirm, onConfirm);
      HANDLE_CASE(Type::OnItemSelect, onItemSelect);
      HANDLE_CASE(Type::OnItemSelectConfirm, onItemSelectConfirm);
      HANDLE_CASE(Type::OnItemDoubleClick, onItemDoubleClick);
      HANDLE_CASE(Type::OnTableOrderingColumnChange, onTableOrderingColumnChange);
      HANDLE_CASE(Type::OnTableSelectionChange, onTableSelectionChange);
      HANDLE_CASE(Type::OnTableHoverChange, onTableHoverChange);
      HANDLE_CASE(Type::OnTableSelectionDoubleClick, onTableSelectionDoubleClick);
      HANDLE_CASE(Type::OnCheck, onCheck);
      HANDLE_CASE(Type::OnCheckChange, onCheckChange);
      HANDLE_CASE(Type::OnNumberInputValueChange, onNumberInputValueChange);
      HANDLE_CASE(Type::OnSwitchToggle, onSwitchToggle);
      HANDLE_CASE(Type::OnSliderMove, onSliderMove);
      HANDLE_CASE(Type::OnMouseEnter, onMouseEnter);
      HANDLE_CASE(Type::OnMouseLeave, onMouseLeave);
      HANDLE_CASE(Type::OnMouseHover, onMouseHover);
      HANDLE_CASE(Type::OnTextEdit, onTextEdit);
      HANDLE_CASE(Type::OnFocusGain, onFocusGain);
      HANDLE_CASE(Type::OnFocusLose, onFocusLose);
      HANDLE_CASE(Type::OnItemDrag, itemDrag);
      HANDLE_CASE(Type::OnMouseDown, onMouseDown);
      HANDLE_CASE(Type::OnMouseUp, onMouseUp);
      HANDLE_CASE(Type::OnMouseMove, onMouseMove);
      HANDLE_CASE(Type::OnMouseDrag, onMouseDrag);
      HANDLE_CASE(Type::OnMouseWheelDown, onMouseWheelDown);
      HANDLE_CASE(Type::OnMouseWheelUp, onMouseWheelUp);
      HANDLE_CASE(Type::OnMouseWheelLeft, onMouseWheelLeft);
      HANDLE_CASE(Type::OnMouseWheelRight, onMouseWheelRight);
      HANDLE_CASE(Type::OnModalMouseDown, onModalMouseDown);
      HANDLE_CASE(Type::OnModalMouseUp, onModalMouseUp);
      HANDLE_CASE(Type::OnKeyDown, onKeyDown);
      HANDLE_CASE(Type::OnKeyUp, onKeyUp);
      HANDLE_CASE(Type::OnKeyRepeat, onKeyRepeat);
      HANDLE_CASE(Type::SelectedTabChanged, onSelectedTabChange);
      HANDLE_CASE(Type::OnCentered, onCenter);
      HANDLE_CASE(Type::OnBeforeDropdownIsShown, onBeforeDropdownIsShown);
      default:;
    }
#undef HANDLE_CASE
  }

  void Listener::initFrom(Listener&& other)
  {
#define HANDLE_CASE(CASE, MEMBER) \
  case CASE: new (&this->MEMBER) decltype(this->MEMBER)(std::move(other.MEMBER)); break
    switch (this->type)
    {
      HANDLE_CASE(Type::OnGuiScaleSetup, onGuiScaleSetup);
      HANDLE_CASE(Type::OnMouseClick, onMouseClick);
      HANDLE_CASE(Type::OnMouseDoubleClick, onMouseDoubleClick);
      HANDLE_CASE(Type::OnToggle, onToggle);
      HANDLE_CASE(Type::OnConfirm, onConfirm);
      HANDLE_CASE(Type::OnItemSelect, onItemSelect);
      HANDLE_CASE(Type::OnItemSelectConfirm, onItemSelectConfirm);
      HANDLE_CASE(Type::OnItemDoubleClick, onItemDoubleClick);
      HANDLE_CASE(Type::OnTableOrderingColumnChange, onTableOrderingColumnChange);
      HANDLE_CASE(Type::OnTableSelectionChange, onTableSelectionChange);
      HANDLE_CASE(Type::OnTableHoverChange, onTableHoverChange);
      HANDLE_CASE(Type::OnTableSelectionDoubleClick, onTableSelectionDoubleClick);
      HANDLE_CASE(Type::OnCheck, onCheck);
      HANDLE_CASE(Type::OnCheckChange, onCheckChange);
      HANDLE_CASE(Type::OnNumberInputValueChange, onNumberInputValueChange);
      HANDLE_CASE(Type::OnSwitchToggle, onSwitchToggle);
      HANDLE_CASE(Type::OnSliderMove, onSliderMove);
      HANDLE_CASE(Type::OnMouseEnter, onMouseEnter);
      HANDLE_CASE(Type::OnMouseLeave, onMouseLeave);
      HANDLE_CASE(Type::OnMouseHover, onMouseHover);
      HANDLE_CASE(Type::OnTextEdit, onTextEdit);
      HANDLE_CASE(Type::OnFocusGain, onFocusGain);
      HANDLE_CASE(Type::OnFocusLose, onFocusLose);
      HANDLE_CASE(Type::OnItemDrag, itemDrag);
      HANDLE_CASE(Type::OnMouseDown, onMouseDown);
      HANDLE_CASE(Type::OnMouseUp, onMouseUp);
      HANDLE_CASE(Type::OnMouseMove, onMouseMove);
      HANDLE_CASE(Type::OnMouseDrag, onMouseDrag);
      HANDLE_CASE(Type::OnMouseWheelDown, onMouseWheelDown);
      HANDLE_CASE(Type::OnMouseWheelUp, onMouseWheelUp);
      HANDLE_CASE(Type::OnMouseWheelLeft, onMouseWheelLeft);
      HANDLE_CASE(Type::OnMouseWheelRight, onMouseWheelRight);
      HANDLE_CASE(Type::OnModalMouseDown, onModalMouseDown);
      HANDLE_CASE(Type::OnModalMouseUp, onModalMouseUp);
      HANDLE_CASE(Type::OnKeyDown, onKeyDown);
      HANDLE_CASE(Type::OnKeyUp, onKeyUp);
      HANDLE_CASE(Type::OnKeyRepeat, onKeyRepeat);
      HANDLE_CASE(Type::SelectedTabChanged, onSelectedTabChange);
      HANDLE_CASE(Type::OnCentered, onCenter);
      HANDLE_CASE(Type::OnBeforeDropdownIsShown, onBeforeDropdownIsShown);
      default:;
    }
#undef HANDLE_CASE
  }

  void Listener::initValue()
  {
#define HANDLE_CASE(CASE, MEMBER) \
  case CASE: new (&MEMBER) decltype(MEMBER); break
    switch (this->type)
    {
      HANDLE_CASE(Type::OnGuiScaleSetup, this->onGuiScaleSetup);
      HANDLE_CASE(Type::OnMouseClick, this->onMouseClick);
      HANDLE_CASE(Type::OnMouseDoubleClick, this->onMouseDoubleClick);
      HANDLE_CASE(Type::OnToggle, this->onToggle);
      HANDLE_CASE(Type::OnConfirm, this->onConfirm);
      HANDLE_CASE(Type::OnItemSelect, this->onItemSelect);
      HANDLE_CASE(Type::OnItemSelectConfirm, this->onItemSelectConfirm);
      HANDLE_CASE(Type::OnItemDoubleClick, this->onItemDoubleClick);
      HANDLE_CASE(Type::OnTableOrderingColumnChange, this->onTableOrderingColumnChange);
      HANDLE_CASE(Type::OnTableSelectionChange, this->onTableSelectionChange);
      HANDLE_CASE(Type::OnTableHoverChange, this->onTableHoverChange);
      HANDLE_CASE(Type::OnTableSelectionDoubleClick, this->onTableSelectionDoubleClick);
      HANDLE_CASE(Type::OnCheck, this->onCheck);
      HANDLE_CASE(Type::OnCheckChange, this->onCheckChange);
      HANDLE_CASE(Type::OnNumberInputValueChange, this->onNumberInputValueChange);
      HANDLE_CASE(Type::OnSwitchToggle, this->onSwitchToggle);
      HANDLE_CASE(Type::OnSliderMove, this->onSliderMove);
      HANDLE_CASE(Type::OnMouseEnter, this->onMouseEnter);
      HANDLE_CASE(Type::OnMouseLeave, this->onMouseLeave);
      HANDLE_CASE(Type::OnMouseHover, this->onMouseHover);
      HANDLE_CASE(Type::OnTextEdit, this->onTextEdit);
      HANDLE_CASE(Type::OnFocusGain, this->onFocusGain);
      HANDLE_CASE(Type::OnFocusLose, this->onFocusLose);
      HANDLE_CASE(Type::OnItemDrag, this->itemDrag);
      HANDLE_CASE(Type::OnMouseDown, this->onMouseDown);
      HANDLE_CASE(Type::OnMouseUp, this->onMouseUp);
      HANDLE_CASE(Type::OnMouseMove, this->onMouseMove);
      HANDLE_CASE(Type::OnMouseDrag, this->onMouseDrag);
      HANDLE_CASE(Type::OnMouseWheelDown, this->onMouseWheelDown);
      HANDLE_CASE(Type::OnMouseWheelUp, this->onMouseWheelUp);
      HANDLE_CASE(Type::OnMouseWheelLeft, this->onMouseWheelLeft);
      HANDLE_CASE(Type::OnMouseWheelRight, this->onMouseWheelRight);
      HANDLE_CASE(Type::OnModalMouseDown, this->onModalMouseDown);
      HANDLE_CASE(Type::OnModalMouseUp, this->onModalMouseUp);
      HANDLE_CASE(Type::OnKeyDown, this->onKeyDown);
      HANDLE_CASE(Type::OnKeyUp, this->onKeyUp);
      HANDLE_CASE(Type::OnKeyRepeat, this->onKeyRepeat);
      HANDLE_CASE(Type::SelectedTabChanged, this->onSelectedTabChange);
      HANDLE_CASE(Type::OnCentered, this->onCenter);
      HANDLE_CASE(Type::OnBeforeDropdownIsShown, this->onBeforeDropdownIsShown);
      default:;
    }
#undef HANDLE_CASE
  }

  void Listener::destroyValue()
  {
    switch (this->type)
    {
      case Type::OnGuiScaleSetup: Util::call_destructor(this->onGuiScaleSetup); break;
      case Type::OnMouseClick: Util::call_destructor(this->onMouseClick); break;
      case Type::OnMouseDoubleClick: Util::call_destructor(this->onMouseDoubleClick); break;
      case Type::OnToggle: Util::call_destructor(this->onToggle); break;
      case Type::OnConfirm: Util::call_destructor(this->onConfirm); break;
      case Type::OnItemSelect: Util::call_destructor(this->onItemSelect); break;
      case Type::OnItemSelectConfirm: Util::call_destructor(this->onItemSelectConfirm); break;
      case Type::OnItemDoubleClick: Util::call_destructor(this->onItemDoubleClick); break;
      case Type::OnTableOrderingColumnChange: Util::call_destructor(this->onTableOrderingColumnChange); break;
      case Type::OnTableSelectionChange: Util::call_destructor(this->onTableSelectionChange); break;
      case Type::OnTableHoverChange: Util::call_destructor(this->onTableHoverChange); break;
      case Type::OnTableSelectionDoubleClick: Util::call_destructor(this->onTableSelectionDoubleClick); break;
      case Type::OnCheck: Util::call_destructor(this->onCheck); break;
      case Type::OnCheckChange: Util::call_destructor(this->onCheckChange); break;
      case Type::OnNumberInputValueChange: Util::call_destructor(this->onNumberInputValueChange); break;
      case Type::OnSwitchToggle: Util::call_destructor(this->onSwitchToggle); break;
      case Type::OnSliderMove: Util::call_destructor(this->onSliderMove); break;
      case Type::OnMouseEnter: Util::call_destructor(this->onMouseEnter); break;
      case Type::OnMouseLeave: Util::call_destructor(this->onMouseLeave); break;
      case Type::OnMouseHover: Util::call_destructor(this->onMouseHover); break;
      case Type::OnTextEdit: Util::call_destructor(this->onTextEdit); break;
      case Type::OnFocusGain: Util::call_destructor(this->onFocusGain); break;
      case Type::OnFocusLose: Util::call_destructor(this->onFocusLose); break;
      case Type::OnItemDrag: Util::call_destructor(this->itemDrag); break;
      case Type::OnMouseDown: Util::call_destructor(this->onMouseDown); break;
      case Type::OnMouseUp: Util::call_destructor(this->onMouseUp); break;
      case Type::OnMouseMove: Util::call_destructor(this->onMouseMove); break;
      case Type::OnMouseDrag: Util::call_destructor(this->onMouseDrag); break;
      case Type::OnMouseWheelDown: Util::call_destructor(this->onMouseWheelDown); break;
      case Type::OnMouseWheelUp: Util::call_destructor(this->onMouseWheelUp); break;
      case Type::OnMouseWheelLeft: Util::call_destructor(this->onMouseWheelLeft); break;
      case Type::OnMouseWheelRight: Util::call_destructor(this->onMouseWheelRight); break;
      case Type::OnModalMouseDown: Util::call_destructor(this->onModalMouseDown); break;
      case Type::OnModalMouseUp: Util::call_destructor(this->onModalMouseUp); break;
      case Type::OnKeyDown: Util::call_destructor(this->onKeyDown); break;
      case Type::OnKeyUp: Util::call_destructor(this->onKeyUp); break;
      case Type::OnKeyRepeat: Util::call_destructor(this->onKeyRepeat); break;
      case Type::SelectedTabChanged: Util::call_destructor(this->onSelectedTabChange); break;
      case Type::OnCentered: Util::call_destructor(this->onCenter); break;
      case Type::OnBeforeDropdownIsShown: Util::call_destructor(this->onBeforeDropdownIsShown); break;
      default:;
    }
  }
}

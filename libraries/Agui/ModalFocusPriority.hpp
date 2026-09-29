#pragma once

namespace agui
{
  enum class ModalFocusPriority : int
  {
    Popup = 30,
    EditorRuntimeMenus = 44,
    MultiplayerTechnologyGui = 45,
    GameStopped = 50,
    SinglePlayerTechnologyGui = 55,
    SpeechBubble = 75,
    Menu = 100,
    DropDown = 200,
    Dialog = 200,
    FloatingWindow = 200
  };
}

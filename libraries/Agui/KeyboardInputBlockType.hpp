#pragma once

namespace agui
{
  enum class KeyboardInputBlockType
  {
    // Block key group from being passed into the game.
    // Special keys are never blocked - e.g. modifiers (ctrl) and text edit keys (backspace)
    None,
    AlphaNumerical, // Printable keys = all keys except the special ones
    NumbersOnly, // numbers and related - "-", "."
    AllowConsole, // exception for some keys to allow closing the console
  };
}

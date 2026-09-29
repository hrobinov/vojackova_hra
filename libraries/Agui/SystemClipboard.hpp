#pragma once
#include <string>

/** Platform clipboard access used by TextBox. Agui only declares these; the back end defines them. */
namespace agui::SystemClipboard
{
  void copy(const std::string& text);
  std::string paste(bool filterNewlines);
}

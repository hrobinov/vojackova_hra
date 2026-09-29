#pragma once
#include "Agui/Widget/ScrollPane.hpp"

namespace agui
{
  class VerticalScrollPane : public ScrollPane
  {
  public:
    VerticalScrollPane(const ScrollPaneStyle* parentStyle = &ScrollPane::defaultStyle);
    virtual ~VerticalScrollPane();
  };
}

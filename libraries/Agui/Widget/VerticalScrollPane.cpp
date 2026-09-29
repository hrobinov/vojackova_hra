#include "Agui/Widget/VerticalScrollPane.hpp"

namespace agui
{
  VerticalScrollPane::VerticalScrollPane(const ScrollPaneStyle* parentStyle)
    : ScrollPane(parentStyle ? parentStyle : &ScrollPane::defaultStyle)
  {
    this->setHScrollPolicy(ScrollPolicy::Never);
    this->style.setHorizontallySquashable(StretchRule::Auto);
  }

  VerticalScrollPane::~VerticalScrollPane()
  {}
}

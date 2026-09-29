#pragma once
#include "Agui/Widget/EmptyWidget.hpp"

namespace agui
{
  class Filler : public EmptyWidget
  {
  public:
    using EmptyWidget::EmptyWidget;
    virtual bool dragOnlyByLeftMouseButton() const override { return true; }
  };
}

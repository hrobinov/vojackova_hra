#pragma once

namespace agui
{
  class ToolTip;

  class ToolTipCreatorBase
  {
  public:
    virtual ToolTip* createToolTip() = 0;
    virtual bool isExtendedStyleViewToolTip() const { return false; }
    virtual ~ToolTipCreatorBase() = default;
  };

  class TooltipCreatorCreator
  {
  public:
    virtual ~TooltipCreatorCreator() = default;
    virtual ToolTipCreatorBase* createCreator() = 0;
    virtual bool equals(const TooltipCreatorCreator&) const = 0;
  };
}

#pragma once
#include "Agui/GenericTargeter.hpp"
#include "Agui/Widget.hpp"
#include <functional>

namespace agui
{
  class Gui;
  class FocusManager;
}

namespace agui
{
  /** Used as the desktop widget in the Gui class. */
  class TopContainer final : public Widget
  {
    using super = Widget;
  public:
    TopContainer(Gui* manager);
    void resizeNextTime(Widget* widget);
    bool processTriggersToResize();
    virtual void postDisplaySizeChanged() override;
    void onResizeFinished();
    virtual void remove(Widget* widget, KeepWidgetAlive keepWidgetAlive = KeepWidgetAlive::False) override;
    void resizeChild(Widget* widget);
    virtual bool interactsWithStyleView() const override { return false; }
    virtual TransparentValue isTransparent() const override { return TransparentValue::Yes; }

    std::function<void()> resizeFinishedCallback;
  private:
    std::vector<GenericTargeter<Widget>> widgetsToResize;
  };
}

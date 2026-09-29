#include "Agui/TopContainer.hpp"
#include "Agui/Gui.hpp"
#include <Agui/Math.hpp>
#include <cassert>

namespace agui
{
  TopContainer::TopContainer(Gui* manager)
  {
    this->setGuiInstanceRecursively(manager);
    this->clearDragEnabled();
  }

  void TopContainer::resizeNextTime(Widget* widget)
  {
    for (auto& item : this->widgetsToResize)
      if (*item == widget)
        return;

    this->widgetsToResize.emplace_back(widget);
  }

  void TopContainer::resizeChild(Widget* widget)
  {
    widget->resizeToContentsRecursive();

    SetSizeInfo setSizeInfo;
    int targetWidth = widget->getWidth();
    int targetHeight = widget->getHeight();
    if (targetWidth > this->getWidth())
    {
      setSizeInfo.horizontal = Change::Squashing;
      targetWidth -= Math::min(widget->maximumHorizontalSquashSize(), targetWidth - this->getWidth());
    }
    if (targetHeight > this->getHeight())
    {
      setSizeInfo.vertical = Change::Squashing;
      targetHeight -= Math::min(widget->maximumVerticalSquashSize(), targetHeight - this->getHeight());
    }
    if (setSizeInfo.squashing())
      widget->setSize(targetWidth, targetHeight, setSizeInfo);
  }

  bool TopContainer::processTriggersToResize()
  {
    if (this->widgetsToResize.empty())
      return false;

    for (GenericTargeter<Widget>& item : this->widgetsToResize)
      if (item && item->getParent() == this)
        this->resizeChild(*item);

    this->widgetsToResize.clear();
    this->onResizeFinished();
    return true;
  }

  void TopContainer::postDisplaySizeChanged()
  {
    Widget::postDisplaySizeChanged();
    this->onResizeFinished();
  }

  void TopContainer::onResizeFinished()
  {
    if (this->resizeFinishedCallback)
      this->resizeFinishedCallback();
  }

  void TopContainer::remove(Widget* widget, KeepWidgetAlive keepWidgetAlive)
  {
    super::remove(widget, keepWidgetAlive);
    this->widgetsToResize.erase(std::remove_if(this->widgetsToResize.begin(),
                                               this->widgetsToResize.end(),
                                               [widget](const GenericTargeter<Widget>& item) { return item == widget; }),
                               this->widgetsToResize.end());
  }
}

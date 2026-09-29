#include "Agui/Graphics.hpp"
#include "Agui/Gui.hpp"
#include "Agui/PaintEvent.hpp"
#include "Agui/Widget/Label.hpp"
#include "Agui/Widget/VerticalFlow.hpp"
#include <cassert>

namespace agui
{
  FrameStyle ToolTip::defaultToolTipStyle;
  FrameStyle ToolTip::defaultToolTipOuterStyle;
  LabelStyle ToolTip::defaultTitleStyle;
  LabelStyle ToolTip::defaultLabelStyle;

  ToolTip::ToolTip(GuiDirection direction, const agui::FrameStyle* parentStyle)
    : Window(direction, parentStyle ? parentStyle : &ToolTip::defaultToolTipStyle)
  {
    Gui::instance->addToolTip(this);
    this->activatedByInstantToolTip = Gui::instance->instantTooltip;
  }

  ToolTip::ToolTip(const agui::FrameStyle* parentStyle)
    : ToolTip(GuiDirection::Vertical, parentStyle)
  {}

  ToolTip::ToolTip(std::string caption, std::string text)
    : agui::Window(agui::GuiDirection::Vertical, &ToolTip::defaultToolTipStyle)
    , caption(std::move(caption))
    , text(std::move(text))
  {
    Gui::instance->addToolTip(this);
    this->activatedByInstantToolTip = Gui::instance->instantTooltip;
  }

  agui::Label* ToolTip::addLabel(const std::string& content, agui::LabelStyle* style, std::optional<agui::Color> colorOverride)
  {
    if (content.empty())
      return nullptr;

    Label* label = new Label(style);
    if (colorOverride)
      label->style.setFontColor(*colorOverride);
    int32_t halfScreenWidth = Gui::instance->getGraphicsContext()->getDisplaySize().width / 2;
    if (label->style.getMaximalWidth() == 0 || label->style.getMaximalWidth() > halfScreenWidth)
      label->style.setMaximalWidth(halfScreenWidth);
    label->setSingleLine(false);
    label->setText(content);
    *this << agui::hold(label);
    return label;
  }

  void ToolTip::updateContent()
  {
    this->clear();
    if (!this->text.empty() || !this->caption.empty())
    {
      this->setVisible(true);
      if (this->text.empty())
        this->addLabel(this->caption, &ToolTip::defaultLabelStyle, this->captionColor);
      else
      {
        this->addLabel(this->caption, &ToolTip::defaultTitleStyle, this->captionColor);
        this->addLabel(this->text, &ToolTip::defaultLabelStyle);
      }
    }
    else
      this->setVisible(false);
  }

  void ToolTip::centerOnMouse()
  {
    if (this->doTooltipLogic())
      this->updatePosition(Gui::instance->getCurrentMousePosition());
  }

  void ToolTip::resizeToContents()
  {
    super::resizeToContents();
    this->centerOnMouse();
  }

  void ToolTip::updatePosition(const agui::Point& mousePosition)
  {
    constexpr int32_t toolTipMonitorEdgeBorder = 10;

    this->recalculateClippingRect();
    const int32_t width = this->clippingRectangle->getWidth();
    const int32_t height = this->clippingRectangle->getHeight();
    const Dimension displaySize = Gui::instance->getGraphicsContext()->getDisplaySize();

    int32_t x;
    int32_t offset = std::max(Gui::instance->getToolTipOffset(), 1);
    if (mousePosition.x + offset <= displaySize.width - width - toolTipMonitorEdgeBorder ||
        width >= (mousePosition.x - offset - toolTipMonitorEdgeBorder - this->clippingRectangle->getLeft()))
      x = mousePosition.x + offset - this->clippingRectangle->getLeft();
    else
      x = std::max(toolTipMonitorEdgeBorder, mousePosition.x - offset - width);

    int32_t y;
    if (mousePosition.y + offset <= displaySize.height - height - toolTipMonitorEdgeBorder)
      y = mousePosition.y + offset - this->clippingRectangle->getTop();
    else
      y = std::max(toolTipMonitorEdgeBorder, mousePosition.y - offset - height);

    this->setLocation(x, y);

    // If this assert is hit it means that ::getWidgetUnderMouse() will return this tooltip.
    // The tooltip should never end up under the mouse because it gets deleted if it does (meaning it never shows).
    assert(!this->intersectionWithPoint(mousePosition));
  }
}

#include "Agui/Widget/TwoLabelsFlow.hpp"
#include "Agui/Widget/Flow.hpp"
#include <algorithm>

namespace agui
{
  TwoLabelsFlow::TwoLabelsFlow(const LabelStyle* leftStype, const LabelStyle* rightStyle)
    : style(this, &Flow::defaultStyle)
    , left(leftStype)
    , right(rightStyle)
  {
    *this << this->left << this->right;
    this->left.setSingleLine(false);
    this->right.setSingleLine(false);
  }

  void TwoLabelsFlow::layoutChildren(SetSizeInfo setSizeInfo)
  {
    if (!setSizeInfo.reactionToSetSize)
    {
      if (this->isHorizontallyStretchable())
        this->setSizeInternal(0, 0);
      else
      {
        this->right.setLocation(this->left.getWidth() + this->style.getHorizontalSpacing(), 0);
        this->setContentSizeInternal(this->right.getRelativeRectangle().getRight(),
                                     std::max(this->left.getHeight(), this->right.getHeight()));
      }
      return;
    }
    this->left.setSize(this->getContentWidth(), 0);
    this->right.setSize(this->getContentWidth(), 0);
    int contentsWidth = this->left.getWidth() + this->style.getHorizontalSpacing() + this->right.getWidth();
    this->left.setLocation(0, 0);
    if (contentsWidth <= this->getContentWidth())
    {
      int y = 0;
      if (this->left.getHeight() > this->right.getHeight())
        y = this->left.getHeight() - this->right.getHeight();
      this->right.setLocation(this->left.getWidth() + this->style.getHorizontalSpacing(), y);
      this->setContentSizeInternal(contentsWidth, std::max(this->left.getHeight(), this->right.getHeight()));
      return;
    }
    this->right.setLocation(this->getContentWidth() - this->right.getWidth(), this->left.getHeight() + this->style.getVerticalSpacing());
    this->setContentSizeInternal(this->getContentWidth(), this->left.getHeight() + this->style.getVerticalSpacing() + this->right.getHeight());
  }
}

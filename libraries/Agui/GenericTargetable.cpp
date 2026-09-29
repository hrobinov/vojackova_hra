#include <Agui/GenericTargetable.hpp>
#include <cassert>

namespace agui
{
  void GenericTargeterBase::attachTo(GenericTargetable* newTarget)
  {
    if (this->target)
    {
      if (this->previous)
        this->previous->next = this->next;
      else
        this->target->first = this->next;
      if (this->next)
        this->next->previous = this->previous;
      this->previous = nullptr;
      this->next = nullptr;
      this->target = nullptr;
    }

    if (newTarget)
    {
      this->next = newTarget->first;
      if (this->next)
        this->next->previous = this;

      this->target = newTarget;
      newTarget->first = this;
    }
  }

  void GenericTargetable::clearTargetingMeGeneric()
  {
    while (this->first)
    {
      assert(this->first->target == this);
      this->first->detach();
    }
  }
}

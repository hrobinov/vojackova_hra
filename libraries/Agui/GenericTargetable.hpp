#pragma once

namespace agui
{
  class GenericTargetable;

  class GenericTargeterBase
  {
  protected:
    ~GenericTargeterBase()
    {
      this->detach();
    }
  public:
    GenericTargeterBase() = default;
    GenericTargeterBase(const GenericTargeterBase&) = delete;
    GenericTargeterBase(GenericTargeterBase&&) = delete;
    GenericTargeterBase& operator=(const GenericTargeterBase&) = delete;
    GenericTargeterBase& operator=(GenericTargeterBase&&) = delete;

  protected:
    GenericTargetable* get() const { return this->target; }
    void detach() { this->attachTo(nullptr); }
    void attachTo(GenericTargetable* newTarget);

  private:
    friend class GenericTargetable;
    GenericTargetable* target = nullptr;
    GenericTargeterBase* previous = nullptr;
    GenericTargeterBase* next = nullptr;
  };

  class GenericTargetable
  {
  protected:
    ~GenericTargetable()
    {
      this->clearTargetingMeGeneric();
    }
  public:
    GenericTargetable() = default;
    GenericTargetable(const GenericTargetable&) : GenericTargetable() {}
    GenericTargetable(GenericTargetable&&) = delete; // Not implemented
    GenericTargetable& operator=(const GenericTargetable&) { return *this; }
    GenericTargetable& operator=(GenericTargetable&&) = delete; // Not implemented

    void clearTargetingMeGeneric();

  private:
    friend class GenericTargeterBase;
    GenericTargeterBase* first = nullptr;
  };
}

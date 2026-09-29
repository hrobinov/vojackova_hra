#pragma once
#include <Agui/GenericTargeter.hpp>
#include <Agui/Listener.hpp>
#include <vector>

namespace agui
{
  class EventDispatchHelper
  {
  public:
    EventDispatchHelper(GenericTargetable* owner, const std::vector<Listener>& listeners, Listener::Type type)
      : guard(owner)
      , listeners(listeners)
      , type(type)
    {}
    constexpr explicit operator bool() const { return bool(this->guard); }

    class Iterator
    {
    public:
      using VectorType = std::vector<Listener>;
      explicit Iterator(EventDispatchHelper& container)
        : container(container)
        , it(this->container.listeners.end())
      {}
      Iterator(EventDispatchHelper& container, bool)
        : container(container)
        , it(this->container.listeners.begin())
      {
        if (this->container.guard)
          this->moveToValid();
        else
          this->it = this->container.listeners.end();
      }

      Listener& operator*() { return *this->it; }
      void operator++()
      {
        if (this->container.guard)
        {
          ++this->it;
          this->moveToValid();
        }
        else
          this->it = this->container.listeners.end();
      }
      bool operator!=(const Iterator& other) const
      {
        return this->it != other.it;
      }
    private:
      void moveToValid()
      {
        while (this->it != this->container.listeners.end() && (!this->it->owner || this->it->type != this->container.type))
          ++this->it;
      }

      EventDispatchHelper& container;
      typename VectorType::iterator it;
    };

    Iterator begin() { return Iterator(*this, true); }
    Iterator end() { return Iterator(*this); }

    GenericTargeter<GenericTargetable> guard;
    std::vector<Listener> listeners;
    Listener::Type type;
  };
}

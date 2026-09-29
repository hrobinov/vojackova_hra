#pragma once
#include <Agui/GenericTargetable.hpp>
#include <type_traits>

namespace agui
{
  class Widget;

  template<class T>
  class GenericTargeter final : public GenericTargeterBase
  {
  public:
    GenericTargeter() = default;
    ~GenericTargeter() = default;
    GenericTargeter(const GenericTargeter& other) { this->attachTo(other.get()); }
    GenericTargeter(T* target) { this->attachTo(target); }
    void clear() { this->detach(); }
    GenericTargeter& operator=(const GenericTargeter& other) { this->attachTo(other.get()); return *this; }
    void operator=(T* target) { this->attachTo(target); }

    constexpr explicit operator bool() const { return bool(this->get()); }
    T* operator->() { return static_cast<T*>(this->get()); }
    const T* operator->() const { return static_cast<const T*>(this->get()); }
    T* operator*() { return static_cast<T*>(this->get()); }
    const T* operator*() const { return static_cast<const T*>(this->get()); }
    bool operator==(const GenericTargeter& other) const { return **this == *other; }
    bool operator!=(const GenericTargeter& other) const { return **this != *other; }
    bool operator==(const Widget* other) const { return **this == other; }
    bool operator!=(const Widget* other) const { return **this != other; }
  };
}

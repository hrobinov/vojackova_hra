#pragma once
#include <utility>

namespace agui
{
  /** Sets a variable for the lifetime of the guard and restores the old value afterwards. */
  template<class T>
  class ScopedSetter
  {
  public:
    explicit ScopedSetter(T& target) : target(target), previous(target) {}
    ScopedSetter(T& target, T value) : target(target), previous(std::move(target)) { this->target = std::move(value); }
    ~ScopedSetter() { this->target = std::move(this->previous); }
    ScopedSetter(const ScopedSetter&) = delete;
    ScopedSetter& operator=(const ScopedSetter&) = delete;
  private:
    T& target;
    T previous;
  };
}

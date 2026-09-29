#pragma once

namespace agui
{
  /** A strongly typed bool, so call sites read f(KeepWidgetAlive::True) rather than f(true). */
  template<class Tag>
  class NamedBool
  {
  public:
    constexpr NamedBool() = default;
    constexpr explicit NamedBool(bool value) : value(value) {}
    constexpr explicit operator bool() const { return this->value; }
    constexpr bool operator!() const { return !this->value; }
    constexpr bool operator==(const NamedBool&) const = default;
    constexpr bool get() const { return this->value; }
    static constexpr NamedBool fromBool(bool value) { return NamedBool(value); }
    static const NamedBool True;
    static const NamedBool False;
  private:
    bool value = false;
  };

  template<class Tag> const NamedBool<Tag> NamedBool<Tag>::True{true};
  template<class Tag> const NamedBool<Tag> NamedBool<Tag>::False{false};
}

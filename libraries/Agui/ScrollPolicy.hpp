#pragma once
#include <cstdint>
#include <string>

namespace agui
{
  class ScrollPolicy
  {
  public:
    // note that the values are serialized in CustomScrollPane, so the order (values) can't be changed without proper migrations.
    enum class Enum : uint8_t
    {
      Never,
      Always,
      Auto,
      AutoAndReserveSpace,
      DontShowButAllowScrolling,
    };
    using enum Enum;

    ScrollPolicy(Enum value) : value(value) {}
    bool operator==(const Enum value) const { return this->value == value; }
    bool operator==(const ScrollPolicy& direction) const { return this->value == direction.value; }
    bool operator!=(const Enum value) const { return this->value != value; }
    bool operator!=(const ScrollPolicy& direction) const { return this->value != direction.value; }
    operator Enum() const { return this->value; }
    static ScrollPolicy parse(std::string_view input);
    const char* str() const;
    uint8_t getIndex() const { return uint8_t(this->value); }

  private:
    Enum value;
  };
}

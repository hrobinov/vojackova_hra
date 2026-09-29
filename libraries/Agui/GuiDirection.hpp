#pragma once
#include <cstdint>
#include <string>
namespace agui
{
  class GuiDirection
  {
  public:
    enum class Enum : uint8_t
    {
      Horizontal = 0,
      Vertical = 1
    };
    using enum Enum;

    GuiDirection(Enum value) : value(value) {}
    bool operator==(const Enum value) const { return this->value == value; }
    bool operator==(const GuiDirection& direction) const { return this->value == direction.value; }
    bool operator!=(const Enum value) const { return this->value != value; }
    bool operator!=(const GuiDirection& direction) const { return this->value != direction.value; }
    operator Enum() const { return this->value; }
    uint8_t getIndex() const { return uint8_t(this->value); }
    Enum asEnum() const { return this->value; }
    std::string str() const;

    static GuiDirection parse(const std::string& value);
  private:
    Enum value;
  };
}

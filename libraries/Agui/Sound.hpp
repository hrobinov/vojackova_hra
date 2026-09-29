#pragma once
#include <string>

namespace agui
{
  class Sound
  {
  public:
    virtual void play(float /*volume*/) const {}
    virtual std::string getName() const = 0;
    virtual ~Sound() {}
  };

  class EmptySound : public Sound
  {
  public:
    virtual std::string getName() const override { return std::string(); }
    static const EmptySound instance;
  };
}

#pragma once
#include <algorithm>
#include <cctype>
#include <string>

namespace agui
{
  class LowercaseString
  {
  public:
    LowercaseString() = default;
    explicit LowercaseString(std::string text) : text(std::move(text))
    {
      std::transform(this->text.begin(), this->text.end(), this->text.begin(),
                     [](unsigned char c) { return char(std::tolower(c)); });
    }
    const std::string& str() const { return this->text; }
    bool empty() const { return this->text.empty(); }
  private:
    std::string text;
  };
}

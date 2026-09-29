#pragma once
#include <cmath>

namespace agui::Math
{
  template<class T> constexpr T min(T a, T b) { return a < b ? a : b; }
  template<class T> constexpr T max(T a, T b) { return a < b ? b : a; }
  template<class T> constexpr T clamp(T value, T low, T high) { return value < low ? low : (high < value ? high : value); }
  template<class T> auto sqrt(T value) { return std::sqrt(value); }
}

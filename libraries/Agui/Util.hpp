#pragma once
#include <algorithm>
#include <iterator>
#include <string>

namespace agui::Util
{
  template<class T> void call_destructor(T& value) { value.~T(); }

  template<class Container, class Predicate>
  bool anyOf(const Container& container, Predicate predicate)
  {
    return std::any_of(std::begin(container), std::end(container), predicate);
  }

  /** MSVC's typeid names are already readable; other compilers get the raw mangled name. */
  inline std::string demangle(const char* name) { return name; }
}

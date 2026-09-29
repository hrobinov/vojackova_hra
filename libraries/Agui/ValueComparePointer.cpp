#include <Agui/Image.hpp>
#include <Agui/ValueComparePointer.hpp>

template<class T>
bool ValueComparePointer<T>::operator==(const ValueComparePointer& other) const
{
  return this->value == other.value || (this->value && other.value && *this->value == *other.value);
}

template struct ValueComparePointer<agui::Image>;

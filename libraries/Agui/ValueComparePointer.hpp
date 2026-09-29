#pragma once

template<class T>
struct ValueComparePointer
{
  ValueComparePointer() = default;
  ValueComparePointer(T* value) : value(value) {}

  T& operator*() const { return *this->value; }
  T* operator->() const { return this->value; }
  operator T*() const { return this->value; }
  operator T*&() { return this->value; }
  explicit operator bool() const { return bool(this->value); }
  bool operator==(const ValueComparePointer& other) const;

  ValueComparePointer& operator=(T* value) { this->value = value; return *this; }

  T* value = nullptr;
};

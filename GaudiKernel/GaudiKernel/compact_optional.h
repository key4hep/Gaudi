#ifndef GAUDIKERNEL_COMPACT_OPTIONAL_H
#define GAUDIKERNEL_COMPACT_OPTIONAL_H 1

#include <stdexcept>

template <typename T, T empty_value>
class compact_optional {
  T m_val;
public:
  constexpr compact_optional() : m_val{empty_value} {}
  constexpr compact_optional(const T& v) : m_val{v} {}
  constexpr compact_optional(T&& v) : m_val{std::move(v)} {}
  constexpr explicit operator bool() const noexcept { return m_val!=empty_value; }
  bool valid() const noexcept { return m_val!=empty_value; }
  const T& value() const noexcept { if ((!bool(*this))) throw std::logic_error("attempt to access empty compact_optional" );
    return m_val; }
  constexpr const T& operator*() const { return m_val; }
  compact_optional& operator=(const T& v) { m_val = v; return *this; }
  compact_optional& operator=(T&& v) { m_val = std::move(v); return *this; } 
  compact_optional& reset() { m_val = empty_value; return *this; }
};


#endif

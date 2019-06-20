#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

// Include files
#include "GaudiKernel/DataObject.h"
#include <cstddef>
#include <iterator>
#include <optional>

namespace details {
  using std::size;

  template <typename T, typename... Args>
  constexpr auto size( const T&, Args&&... ) noexcept {
    static_assert( sizeof...( Args ) == 0, "No extra args please" );
    return std::nullopt;
  }
} // namespace details

// ugly hack to circumvent the usage of std::any
struct GAUDI_API AnyDataWrapperBase : DataObject {
  virtual std::optional<std::size_t> size() const = 0;
};

template <class T>
class GAUDI_API AnyDataWrapper final : public AnyDataWrapperBase {
public:
  AnyDataWrapper( T&& data ) : m_data( std::move( data ) ){};

  AnyDataWrapper( AnyDataWrapper&& other )
      : AnyDataWrapperBase( std::move( other ) ), m_data( std::move( other.m_data ) ){};

  const T& getData() const { return m_data; }
  T&       getData() { return m_data; }

  std::optional<std::size_t> size() const override {
    using details::size;
    return size( m_data );
  }

private:
  T m_data;
};

#endif

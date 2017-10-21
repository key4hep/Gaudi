#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

// Include files
#include "GaudiKernel/DataObject.h"

namespace details
{
  template <typename T>
  auto size( const T& t ) -> decltype( t.size() )
  {
    return t.size();
  }

  template <typename T, typename... Args>
  constexpr int size( const T&, Args&&... )
  {
    static_assert( sizeof...( Args ) == 0, "No extra args please" );
    return -1;
  }
}

// ugly hack to circumvent the usage of boost::any
struct GAUDI_API AnyDataWrapperBase : DataObject {
  virtual int size() const = 0;
};

template <class T>
class GAUDI_API AnyDataWrapper final : public AnyDataWrapperBase
{
public:
  AnyDataWrapper( T&& data ) : m_data( std::move( data ) ){};

  AnyDataWrapper( AnyDataWrapper&& other )
      : AnyDataWrapperBase( std::move( other ) ), m_data( std::move( other.m_data ) ){};

  const T& getData() const { return m_data; }
  T& getData() { return m_data; }

  int size() const override { return details::size( m_data ); }

private:
  T m_data;
};

#endif

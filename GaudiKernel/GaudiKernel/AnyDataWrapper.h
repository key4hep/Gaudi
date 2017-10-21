#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

// Include files
#include "GaudiKernel/DataObject.h"

struct GAUDI_API AnyDataWrapperBase : DataObject {
  // ugly hack to circumvent the usage of boost::any yet
  // DataSvc would need a templated register method
  virtual int size() const = 0;
};

namespace details
{

  template <typename>
  struct void_t {
    using type = void;
  };
  template <typename T>
  using is_valid_t = typename void_t<T>::type;

  template <typename T, typename SFINAE = void>
  struct has_size_method : std::false_type {
  };

  template <typename T>
  struct has_size_method<T, is_valid_t<decltype( std::declval<const T&>().size() )>> : std::true_type {
  };

  template <typename T>
  int size( const T& t, std::true_type )
  {
    return t.size();
  }

  template <typename T>
  int size( const T&, std::false_type )
  {
    return -1;
  }
}

template <class T>
class GAUDI_API AnyDataWrapper final : public AnyDataWrapperBase
{
public:
  AnyDataWrapper( T&& data ) : m_data( std::move( data ) ){};

  AnyDataWrapper( AnyDataWrapper&& other )
      : AnyDataWrapperBase( std::move( other ) ), m_data( std::move( other.m_data ) ){};

  const T& getData() const { return m_data; }
  T& getData() { return m_data; }

  int size() const override { return details::size( m_data, details::has_size_method<T>{} ); }

private:
  T m_data;
};

#endif

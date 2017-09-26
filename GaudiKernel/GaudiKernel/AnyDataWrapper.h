#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

// Include files
#include "GaudiKernel/DataObject.h"

struct GAUDI_API AnyDataWrapperBase : DataObject {
  // ugly hack to circumvent the usage of boost::any yet
  // DataSvc would need a templated register method
  virtual ~AnyDataWrapperBase(){};
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

private:
  T m_data;
};

#endif

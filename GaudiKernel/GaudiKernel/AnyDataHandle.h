#ifndef GAUDIKERNEL_ANYDATAHANDLE_H
#define GAUDIKERNEL_ANYDATAHANDLE_H

#include "GaudiKernel/AnyDataWrapper.h"
#include <GaudiKernel/DataObjectHandle.h>

// TODO: we should derive directly from DataObjectHandleBase
//      now that that is sufficient to get all the declareProperty
//      stuff done... (thanks to the SFINAE constraints in
//      Algortihm and AlgTool!)
//      And then we can avoid any confusion with the DataObjectHandle
//      interface.
template <typename T>
struct AnyDataHandle final : public DataObjectHandle<AnyDataWrapper<T>> {

  using DataObjectHandle<AnyDataWrapper<T>>::DataObjectHandle;

  /**
   * Retrieve object from transient data store
   */
  const T* get() const;

  /**
   * Register object in transient store
   */
  const T* put( T&& object );
};

//---------------------------------------------------------------------------

template <typename T>
const T* AnyDataHandle<T>::get() const
{
  auto doh = DataObjectHandle<AnyDataWrapper<T>>::get();
  return &( doh->getData() );
}

//---------------------------------------------------------------------------

template <typename T>
const T* AnyDataHandle<T>::put( T&& objectp )
{
  auto dw = new AnyDataWrapper<T>( std::move( objectp ) );
  return &DataObjectHandle<AnyDataWrapper<T>>::put( dw )->getData();
}

#endif

#ifndef GAUDIKERNEL_ANYDATAHANDLE_H
#define GAUDIKERNEL_ANYDATAHANDLE_H

#include <type_traits>


#include "GaudiKernel/AnyDataWrapper.h"
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/Range.h>
#include <GaudiKernel/NamedRange.h>

// ============================================================================
// Default implementation of the AnyDataHandle
// ============================================================================

//TODO: we should derive directly from DataObjectHandleBase
//      now that that is sufficient to get all the declareProperty
//      stuff done... (thanks to the SFINAE constraints in
//      Algortihm and AlgTool!)
//      And then we can avoid any confusion with the DataObjectHandle
//      interface.
template<typename T>
struct AnyDataHandle final : public DataObjectHandle<AnyDataWrapper<T> > {

  using DataObjectHandle<AnyDataWrapper<T>>::DataObjectHandle;

  /**
   * Retrieve object from transient data store
   */
  const T* get() const;

  /**
   * Register object in transient store
   */
  const T* put(T&& object);
};

//---------------------------------------------------------------------------

template<typename T>
const T* AnyDataHandle<T>::get() const {
  auto doh = DataObjectHandle<AnyDataWrapper<T>>::get();
  return &(doh->getData());
}

//---------------------------------------------------------------------------

template<typename T>
const T* AnyDataHandle<T>::put (T&& objectp){
  auto dw = new AnyDataWrapper<T>(std::move(objectp));
  return &DataObjectHandle<AnyDataWrapper<T> >::put(dw)->getData();
}

//----------------------------------------------------------------------------
// Specialization for NamedRanges

template<typename T>
struct AnyDataHandle<Gaudi::NamedRange_<T>> final : 
  public DataObjectHandle<Gaudi::NamedRange_<T>> {

  using DataObjectHandle<Gaudi::NamedRange_<T>>::DataObjectHandle;

  /**
   * Retrieve object from transient data store
   */
  const Gaudi::NamedRange_<T> get() const;

};

template<typename T>
const Gaudi::NamedRange_<T> AnyDataHandle<Gaudi::NamedRange_<T>>::get() const {
  return DataObjectHandle<Gaudi::NamedRange_<T>>::get();
}

//---------------------------------------------------------------------------
// Specialization for Ranges

template<typename T>
struct AnyDataHandle<Gaudi::Range_<T>> final : 
  public DataObjectHandle<Gaudi::Range_<T>> {

  using DataObjectHandle<Gaudi::Range_<T>>::DataObjectHandle;

  /**
   * Retrieve object from transient data store
   */
  const Gaudi::Range_<T> get() const;

};

template<typename T>
const Gaudi::Range_<T> AnyDataHandle<Gaudi::Range_<T>>::get() const {
  return DataObjectHandle<Gaudi::Range_<T>>::get();
}


#endif

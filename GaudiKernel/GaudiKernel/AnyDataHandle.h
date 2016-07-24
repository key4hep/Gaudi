#ifndef GAUDIKERNEL_ANYDATAHANDLE_H
#define GAUDIKERNEL_ANYDATAHANDLE_H

#include "GaudiKernel/AnyDataWrapper.h"

#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/ServiceLocatorHelper.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Property.h>
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"

template<typename T>
class AnyDataHandle : public DataObjectHandle<AnyDataWrapper<T> > {

public:
  friend class Algorithm;
  friend class AlgTool;

public:

  using DataObjectHandle<AnyDataWrapper<T>>::DataObjectHandle;

  /**
   * Retrieve object from transient data store
   */
  const T* get() const;

  /**
   * Register object in transient store
   */
  const T* put (T&& object);

};

//---------------------------------------------------------------------------                                           

/**
 * Try to retrieve from the transient store. If the retrieval succeded and 
 * this is the first time we retrieve, perform a dynamic cast to the desired 
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the 
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template<typename T>  
const T* AnyDataHandle<T>::get() const {
  auto doh = DataObjectHandle<AnyDataWrapper<T>>::get();
  return &(doh->getData());
}

template<typename T>  
const T* AnyDataHandle<T>::put (T&& objectp){

  AnyDataWrapper<T>* dw = new AnyDataWrapper<T>(std::move(objectp));  
  return &DataObjectHandle<AnyDataWrapper<T> >::put(dw)->getData();

}


#endif

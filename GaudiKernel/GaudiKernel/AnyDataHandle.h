#ifndef GAUDIKERNEL_ANYDATAHANDLE_H
#define GAUDIKERNEL_ANYDATAHANDLE_H

#include <type_traits>

#include "GaudiKernel/AnyDataWrapper.h"
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/Range.h>
#include <GaudiKernel/NamedRange.h>


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

  
  //const typename T::value_type::Range getr() const;

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

//---------------------------------------------------------------------------

/*
 * Template specialization for AnyDataHandle of Ranges.
 * We can get one, but not put it in...
 */
template<typename TT>
struct AnyDataHandle<Gaudi::NamedRange_<TT>>  : 
  public DataObjectHandle<AnyDataWrapper<TT>> {

public:

  using DataObjectHandle<AnyDataWrapper<TT>>::DataObjectHandle;

  const Gaudi::NamedRange_<TT> get() const {
    try {
      auto m = DataObjectHandle<AnyDataWrapper<TT>>::get();
      return Gaudi::NamedRange_<TT>(m->getData().begin(), 
                                    m->getData().end());
    } catch(const std::exception &exc) {

      std::cout << "Got exception " << exc.what() << std::endl;
      
    }  
    return Gaudi::NamedRange_<TT>();
  }
};

#endif

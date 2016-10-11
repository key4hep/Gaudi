#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/GetDataHelpers.h"
#include "GaudiKernel/AnyDataWrapper.h"


#include <type_traits>

//---------------------------------------------------------------------------

/** DataObjectHandle.h GaudiKernel/DataObjectHandle.h
 *
 * Templated Handle class for objects in the event store
 *
 * Inheritance: DataHandle->DataObjectHandleBase->DataObjectHandle<T>
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------


template<typename T>
class DataObjectHandle : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  T* get() const { return get(true); }

  /**
   * Bypass check of existence of object in transient store
   * Only uses main location of the
   */
  T* getIfExists() const { return get(false); }

  /**
   * Check the existence of the object in the transient store
   */
  bool exist() const { return get(false) != nullptr; }

  /**
   * Get object from store or create a new one if it doesn't exist
   */
  T* getOrCreate();

  /**
   * Register object in transient store
   */
  T* put (T* object);

private:

  T* get(bool mustExist) const;
  mutable bool  m_goodType = false;

  Gaudi::Helpers::GetData<T> m_adapter;

};

//---------------------------------------------------------------------------
//
/**
 * Try to retrieve from the transient store. If the retrieval succeded and
 * this is the first time we retrieve, perform a dynamic cast to the desired
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template<typename T>
T* DataObjectHandle<T>::get(bool mustExist) const {

  auto dataObj = fetch();

  if (UNLIKELY(!dataObj) ) {
    if (mustExist) { // Problems in getting from the store
        throw GaudiException("Cannot retrieve " + objKey() +
                             " from transient store.",
                             m_owner ? owner()->name() : "no owner",
                             StatusCode::FAILURE);
    }
    return nullptr;
  }
  
  // Using the 
  return m_adapter(dataObj);
}

//---------------------------------------------------------------------------
template<typename T>
T* DataObjectHandle<T>::put (T *objectp){
  assert(m_init);
  StatusCode rc = m_EDS->registerObject(objKey(), objectp);
  if (!rc.isSuccess()) {
    throw GaudiException("Error in put of " + objKey(), 
                         "DataObjectHandle<T>::put",
                         StatusCode::FAILURE);
  }
  return objectp;
}

//---------------------------------------------------------------------------
template<typename T>
T* DataObjectHandle<T>::getOrCreate (){

	//this process needs to be locking for multi-threaded applications
	//lock(); --> done in caller

	T* obj = get(false);

	//object exists, we are done
	if(obj){
		//unlock();
		return obj;
	}

	//create it
	return put(new T{});

	//unlock();
}

//---------------------------------------------------------------------------
// Specialization for NamedRanges, in that case, we return it by value

template<typename T>
class DataObjectHandle<Gaudi::NamedRange_<T>> : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  Gaudi::NamedRange_<T> get() const { return get(true); }

private:

  Gaudi::NamedRange_<T> get(bool mustExist) const;

  Gaudi::Helpers::GetData<Gaudi::NamedRange_<T>> m_adapter;

};

template<typename T>
Gaudi::NamedRange_<T> DataObjectHandle<Gaudi::NamedRange_<T>>::get(bool mustExist) const {

  auto dataObj = fetch();

  if (UNLIKELY(!dataObj) ) {
    if (mustExist) { // Problems in getting from the store
        throw GaudiException("Cannot retrieve " + objKey() +
                             " from transient store.",
                             m_owner ? owner()->name() : "no owner",
                             StatusCode::FAILURE);
    }
    return Gaudi::NamedRange_<T>();
  }
  
  return m_adapter(dataObj);
}

//---------------------------------------------------------------------------
// Specialization for Ranges, in that case, we return it by value

template<typename T>
class DataObjectHandle<Gaudi::Range_<T>> : public DataObjectHandleBase {
public:
  using DataObjectHandleBase::DataObjectHandleBase;

  /**
   * Retrieve object from transient data store
   */
  Gaudi::Range_<T> get() const { return get(true); }

private:

  Gaudi::Range_<T> get(bool mustExist) const;

  Gaudi::Helpers::GetData<Gaudi::Range_<T>> m_adapter;

};

template<typename T>
Gaudi::Range_<T> DataObjectHandle<Gaudi::Range_<T>>::get(bool mustExist) const {

  auto dataObj = fetch();

  if (UNLIKELY(!dataObj) ) {
    if (mustExist) { // Problems in getting from the store
        throw GaudiException("Cannot retrieve " + objKey() +
                             " from transient store.",
                             m_owner ? owner()->name() : "no owner",
                             StatusCode::FAILURE);
    }
    return Gaudi::Range_<T>();
  }
  
  return m_adapter(dataObj);
}


//------------------------------------------------------------------------


#endif

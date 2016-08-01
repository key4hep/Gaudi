#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"

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

  if (UNLIKELY(!m_goodType)) { // Check type compatibility once

    T* obj = dynamic_cast<T*>(dataObj);
    m_goodType = ( obj!=nullptr );

    if (UNLIKELY(!m_goodType)) {

      std::string errorMsg("The type provided for "+ objKey()
                           + " is " + typeid(T).name()
                           + " and is different form the one of the object in the store.");
      //log << MSG::ERROR << errorMsg << endmsg;
      throw GaudiException (errorMsg,"Wrong DataObjectType",StatusCode::FAILURE);
    }
    //log << MSG::DEBUG <<  "The data type (" <<  typeid(T).name()
    //    << ") specified for the handle of " << dataProductName()
    //    << " is the same of the object in the store. "
    //    << "From now on the result of a static_cast will be returned." << endmsg;
    return obj;
  }

  //  setRead();
  // From the second read on, this is safe
  return static_cast<T*> (dataObj);
}

//---------------------------------------------------------------------------
template<typename T>
T* DataObjectHandle<T>::put (T *objectp){

  if (UNLIKELY(!m_init)) init();

  m_EDS->registerObject(objKey(), objectp).ignore();
  // if ( LIKELY( sc.isSuccess() ) )
  //   setWritten();
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

	//MsgStream log(m_MS,"DataObjectHandle");
    //log << MSG::DEBUG << "Object " << objKey() << " does not exist, creating it" << endmsg;

	//create it
	return put(new T{});

	//unlock();
}


#endif

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

  DataObjectHandle();
  DataObjectHandle(const DataObjID& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o);
  DataObjectHandle(const std::string& k, Gaudi::DataHandle::Mode a,
		   IDataHandleHolder* o);
  
  virtual ~DataObjectHandle() {}

  /**
   * Retrieve object from transient data store
   */
  T* get() { return get(true); }

  /**
   * Bypass check of existence of object in transient store
   * Only uses main location of the
   */
  T* getIfExists() { return get(false); }

  /**
   * Check the existence of the object in the transient store
   */
  bool exist() { return get(false) != NULL; }

  /**
   * Get object from store or create a new one if it doesn't exist
   */
  T* getOrCreate();

  /**
   * Register object in transient store
   */
  void put (T* object);

private:

  T* get(bool mustExist);

};

//---------------------------------------------------------------------------

template<typename T>
DataObjectHandle<T>::DataObjectHandle():
  DataObjectHandleBase() {}

//---------------------------------------------------------------------------
template<typename T>
DataObjectHandle<T>::DataObjectHandle(const DataObjID & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  DataObjectHandleBase(k,a,owner) {}

template<typename T>
DataObjectHandle<T>::DataObjectHandle(const std::string & k,
				      Gaudi::DataHandle::Mode a,
				      IDataHandleHolder* owner):
  DataObjectHandleBase(k,a,owner) {} 

//---------------------------------------------------------------------------

/**
 * Try to retrieve from the transient store. If the retrieval succeded and
 * this is the first time we retrieve, perform a dynamic cast to the desired
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template<typename T>
T* DataObjectHandle<T>::get(bool mustExist) {

  if (!m_init) {
    init();
  }

  //MsgStream log(m_MS,"DataObjectHandle");

  DataObject* dataObjectp = NULL;

  StatusCode sc = m_EDS->retrieveObject(objKey(), dataObjectp);

  T* returnObject = NULL;
  if ( LIKELY( sc.isSuccess() ) ){

    if (UNLIKELY(!m_goodType)){ // Check type compatibility once

      // DP: can use a gaudi feature?
      m_goodType = (NULL != dynamic_cast<T*> (dataObjectp));
      //( typeid(tmp) == typeid(*dataObjectp) ) ;

      //T tmp;
      //const std::string dataType(typeid(tmp).name());
      const std::string dataType(typeid(T).name());

      if (!m_goodType){
        std::string errorMsg("The type provided for "+ objKey()
                             + " is " + dataType
                             + " and is different form the one of the object in the store.");
        //log << MSG::ERROR << errorMsg << endmsg;
        throw GaudiException (errorMsg,"Wrong DataObjectType",StatusCode::FAILURE);
      }
      else{
        //log << MSG::DEBUG <<  "The data type (" <<  dataType
        //    << ") specified for the handle of " << dataProductName()
        //    << " is the same of the object in the store. "
        //    << "From now on the result of a static_cast will be returned." << endmsg;
      }
    }

    if (LIKELY(m_goodType)) { // From the second read on, this is safe
      returnObject = static_cast<T*> (dataObjectp);
    }

  }
  else if(mustExist){ // Problems in getting from the store
    throw GaudiException("Cannot retrieve " + objKey() + 
			 " from transient store.",
			 m_owner != 0 ? owner()->name() : "no owner",
			 StatusCode::FAILURE);
  }

  //  setRead();
  return returnObject;
}

//---------------------------------------------------------------------------
template<typename T>
void DataObjectHandle<T>::put (T *objectp){

  if (!m_init) {
    init();
  }

  StatusCode sc = m_EDS->registerObject(objKey(), objectp);
  sc.ignore();
  // if ( LIKELY( sc.isSuccess() ) )
  //   setWritten();    
}

//---------------------------------------------------------------------------
template<typename T>
T* DataObjectHandle<T>::getOrCreate (){

	//this process needs to be locking for multi-threaded applications
	//lock(); --> done in caller

	T* obj = get(false);

	//object exists, we are done
	if(obj != NULL){

		//unlock();
		return obj;
	}

	//MsgStream log(m_MS,"DataObjectHandle");
  //log << MSG::DEBUG << "Object " << objKey() << " does not exist, creating it" << endmsg;

	//create it
	obj = new T();
	put(obj);

	//unlock();
	return obj;
}

                    
#endif

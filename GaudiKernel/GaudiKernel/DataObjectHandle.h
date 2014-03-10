#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include <GaudiKernel/MinimalDataObjectHandle.h>
#include <GaudiKernel/ServiceLocatorHelper.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Property.h>
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"

template<typename T>
class DataObjectHandle : public MinimalDataObjectHandle {

public:
      
  /// Initialises mother class
  DataObjectHandle(const std::string& productName,
                   IAlgorithm* fatherAlg,
                   AccessType accessType = READ,
                   bool isOptional=false);
  
  /// Initialises mother class
  DataObjectHandle(const std::string& productName,
                   IAlgTool* fatherTool,
                   AccessType accessType = READ,
                   bool isOptional=false);

  /// Initialize
  StatusCode initialize();

  /// Reinitialize
  StatusCode reinitialize();  
  
  /// Finalize
  StatusCode finalize();    
 
  /// Get from the transient store using the processed event number to identify the correct slot
  T* get() ;
  
  /// Register from the transient store using the processed event number to identify the correct slot
  void put (T* object);
  
private:
  bool m_goodType;
  SmartIF<IDataProviderSvc> m_EDS;
  SmartIF<IMessageSvc> m_MS;
  IAlgorithm* m_fatherAlg;
  IAlgTool* m_fatherTool;
  DataObjectHandle(const DataObjectHandle& );
  DataObjectHandle& operator=(const DataObjectHandle& );  
  
};

//---------------------------------------------------------------------------
template<typename T>
StatusCode DataObjectHandle<T>::initialize(){

  // GCCXML cannot understand c++11 yet, NULL used.

  MinimalDataObjectHandle::initialize();

  if (m_fatherAlg != 0) {
		// Fetch the event Data Service from the algorithm
		Algorithm* algorithm = dynamic_cast<Algorithm*>(m_fatherAlg);
		if (LIKELY(algorithm != NULL)) {
			m_EDS = algorithm->evtSvc();
			m_MS = algorithm->msgSvc();
		} else
			throw GaudiException(
					"Cannot cast " + m_fatherAlg->name() + " to Algorithm.",
					"Invalid Cast", StatusCode::FAILURE);
	}

	if (m_fatherTool != 0) {
		// Fetch the event Data Service from the algorithm
		AlgTool* tool = dynamic_cast<AlgTool*>(m_fatherTool);
		if (LIKELY(tool != NULL)) {
			m_EDS = tool->evtSvc();
			m_MS = tool->msgSvc();
		} else
			throw GaudiException(
					"Cannot cast " + m_fatherTool->name() + " to AlgTool.",
					"Invalid Cast", StatusCode::FAILURE);
	}

   m_goodType = false;

   return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
template<typename T>
StatusCode DataObjectHandle<T>::reinitialize(){

  MinimalDataObjectHandle::reinitialize();
  
  m_goodType = false;
  
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
template<typename T>
StatusCode DataObjectHandle<T>::finalize(){

  MinimalDataObjectHandle::finalize();
  
  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
template<typename T>
DataObjectHandle<T>::DataObjectHandle(const std::string& productName,
                                      IAlgorithm* fatherAlg,
                                      IDataObjectHandle::AccessType accessType,
                                      bool isOptional):
                   MinimalDataObjectHandle(productName,
                                           accessType,
                                           isOptional),
                                           m_fatherAlg(fatherAlg),
                                           m_fatherTool(0),
                                           m_goodType(false){}

//---------------------------------------------------------------------------
template<typename T>
DataObjectHandle<T>::DataObjectHandle(const std::string& productName,
                                      IAlgTool* fatherTool,
                                      IDataObjectHandle::AccessType accessType,
                                      bool isOptional):
                   MinimalDataObjectHandle(productName,
                                           accessType,
                                           isOptional),
                                           m_fatherAlg(0),
                                           m_fatherTool(fatherTool),
                                           m_goodType(false){}

//---------------------------------------------------------------------------                                           

/**
 * Try to retrieve from the transient store. If the retrieval succeded and 
 * this is the first time we retrieve, perform a dynamic cast to the desired 
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the 
 * static cast: we do not need the checks of the dynamic cast for every access!
 */
template<typename T>  
T* DataObjectHandle<T>::get() {

	MsgStream log(m_MS,"DataObjectHandle");

  DataObject* dataObjectp = NULL;

  StatusCode sc = m_EDS->retrieveObject(m_dataProductName, dataObjectp);
  
  if(sc.isSuccess())
	  log << MSG::DEBUG << "Using main location " << m_dataProductName << " for " << *dataObjectp << endmsg;

  if(sc.isFailure() && ! m_alternativeDataProducts.empty()){
	  for(uint i = 0; i < m_alternativeDataProducts.size() && sc.isFailure(); ++i){
		  sc = m_EDS->retrieveObject(m_alternativeDataProducts[i], dataObjectp);

		  if(sc.isSuccess())
		  	  log << MSG::DEBUG << "Using alternative location " << m_alternativeDataProducts[i] << " for " << *dataObjectp << endmsg;
	  }
  }

  T* returnObject = NULL;
  if ( LIKELY( sc.isSuccess() ) ){ 
    
    if (UNLIKELY(!m_goodType)){ // Check type compatibility once

      // DP: can use a gaudi feature?
      m_goodType = (NULL != dynamic_cast<T*> (dataObjectp));
      //( typeid(tmp) == typeid(*dataObjectp) ) ;

      T tmp;
      
      const std::string dataType(typeid(tmp).name());
      
      if (!m_goodType){
        std::string errorMsg("The type provided for "+ m_dataProductName 
                             + " is " + dataType
                             + " and is different form the one of the object in the store.");
        log << MSG::ERROR << errorMsg << endmsg;        
        throw GaudiException (errorMsg,"Wrong DataObjectType",StatusCode::FAILURE);                
      }
      else{
        log << MSG::DEBUG <<  "The data type (" <<  dataType
            << ") specified for the handle of " << m_dataProductName
            << " is the same of the object in the store. "
            << "From now on the result of a static_cast will be returned." << endmsg;
      }
    }
    
    if (LIKELY(m_goodType)) { // From the second read on, this is safe
      returnObject = static_cast<T*> (dataObjectp);
    }

  }
  else{ // Problems in getting from the store
    MsgStream log(m_MS,"DataObjectHandle");
    log << MSG::ERROR << "Cannot retrieve " 
        << m_dataProductName << " from transient store. "
        << "As a result, a segmentation fault is very likely." << endmsg;
    return NULL;
  }
  
  setRead();
  return returnObject;  
}
  
//---------------------------------------------------------------------------
template<typename T>  
void DataObjectHandle<T>::put (T *objectp){
    
  
    StatusCode sc = m_EDS->registerObject(m_dataProductName, objectp);
    if ( LIKELY( sc.isSuccess() ) )
    setWritten();    
}
                                           
#endif

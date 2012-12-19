#ifndef GAUDIHIVE_DATAOBJECTHANDLE_H
#define GAUDIHIVE_DATAOBJECTHANDLE_H

#include <GaudiKernel/MinimalDataObjectHandle.h>
//#include <GaudiAlg/GaudiCommon.h>
#include <GaudiKernel/ServiceLocatorHelper.h>
#include <GaudiKernel/GaudiException.h>
#include "GaudiKernel/Algorithm.h"

template<typename T>
class DataObjectHandle : public MinimalDataObjectHandle {

public:
      
  /// Initialises mother class
  DataObjectHandle(const std::string& productName,
                   IAlgorithm* fatherAlg,
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
  DataObjectHandle(const DataObjectHandle& );
  DataObjectHandle& operator=(const DataObjectHandle& );  
  
};

//---------------------------------------------------------------------------
template<typename T>
StatusCode DataObjectHandle<T>::initialize(){

  // GCCXML cannot understand c++11 yet, NULL used.
  
  MinimalDataObjectHandle::initialize();
  
  // Fetch the event Data Service from the algorithm
   Algorithm* algorithm = dynamic_cast<Algorithm*>(m_fatherAlg);
   if (LIKELY(algorithm != NULL)){
     m_EDS = algorithm->eventSvc();
     m_MS = algorithm->msgSvc();
    }
   else
     throw GaudiException("Cannot cast " + m_fatherAlg->name() + " to Algorithm.",
                    "Invalid Cast",
                    StatusCode::FAILURE);         
     
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
                                           fatherAlg,
                                           accessType,
                                           isOptional),
                                           m_goodType(false){}

//---------------------------------------------------------------------------                                           

// As in GaudiAlgorithm
/**
 * Try to retrieve from the transient store. If the retrieval succeded and 
 * this is the first time we retrieve, perform a dynamic cast to the desired 
 * object. Then finally set the handle as Read.
 * If this is not the first time we cast and the cast worked, just use the 
 * plain C cast (explicit conversion): we do not need the checks of the 
 * dynamic cast!
 */
template<typename T>  
T* DataObjectHandle<T>::get() {

  DataObject* dataObjectp = NULL;
  StatusCode sc = m_EDS->retrieveObject(m_productName, dataObjectp);
  
  T* returnObject = NULL;
  if ( LIKELY( sc.isSuccess() ) ){ 
    
    if (UNLIKELY(!m_goodType)){ // Check type compatibility once
      MsgStream log(m_MS,"DataObjectHandle");
      T tmp;
      
      m_goodType = ( typeid(tmp) == typeid(*dataObjectp) ) ;

      if (! m_goodType){
        std::string errorMsg("The type provided for "+ m_productName 
                        + " is " + std::string(typeid(tmp).name()) 
                        + " and is different form the one of the object in the store.");
        log << MSG::ERROR << errorMsg << endmsg;        
        throw GaudiException (errorMsg,"Wrong DataObjectType",StatusCode::FAILURE);                
      }
      else{
        log << MSG::INFO <<  "The data type specified for the handle of " 
            << m_productName << " is the same of the object in the store. "
            << "From now on a reinterpret_cast will be performed." << endmsg;
      }
    }
    
    if (LIKELY(m_goodType)) { // From the second read on, this is safe
      returnObject = reinterpret_cast<T*> (dataObjectp);       
    }

  }
  else{ // Problems in getting from the store
    MsgStream log(m_MS,"DataObjectHandle");
    log << MSG::ERROR << "Cannot retrieve " 
        << m_productName << " from transient store." << endmsg;
  }
  
  setRead();        
  return returnObject;  
}
  
//---------------------------------------------------------------------------
template<typename T>  
void DataObjectHandle<T>::put (T *objectp){
    
  
    StatusCode sc = m_EDS->registerObject(m_productName, objectp);
    if ( LIKELY( sc.isSuccess() ) )
    setWritten();    
}
                                           
#endif
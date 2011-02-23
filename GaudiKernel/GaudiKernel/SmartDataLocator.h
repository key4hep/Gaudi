//	====================================================================
//	SmartDataLocator.h
//	--------------------------------------------------------------------
//
//	Package    : GaudiKernel ( The LHCb Offline System)
//
//  Description: Implementation of a smart pointer class to access
//               easily (and efficiently) data stores.
//
//	Author     : M.Frank
//	====================================================================
#ifndef GAUDIKERNEL_SMARTDATALOCATOR_H
#define GAUDIKERNEL_SMARTDATALOCATOR_H 1

// Framework include files
#include "GaudiKernel/SmartDataPtr.h"

/** A small class used to access easily (and efficiently) data items
    residing in data stores.

      The class constructors take several arguments neccessary to be passed
    tyo the data services in order to automatically load objects in case 
    they are not yet loaded. This is achieved through a smart pointer 
    mechanism i.e. by overloading the operator->() at dereferencing time 
    the the object will be requested from the store.

      The SmartDataLocator is meant to be "short living". It only makes 
    sense to keep an object instance within e.g. the scope of one method.
    "long living" instances do not make sense and in the contrary
    would be harmful, because the information passed during construction
    to would be invalid and returned object pointers would actually
    point to hyperspace.

    The intrinsic functionality, wether the object will be retrieved or
    loaded from the data store is defined by the LOADER::.


    Base Class:
    SmartDataStorePtr

    @author  M.Frank
    @version 1.0
*/

template<class TYPE> class SmartDataLocator : public SmartDataStorePtr<TYPE, SmartDataObjectPtr::ObjectLoader>    {
public:
  /** Standard constructor: Construct an SmartDataLocator instance which is 
                            able to connect to a DataObject instance
                            which is identified by its parent object and
                            the path relative to the parent.
      @param  pService      Pointer to the data service interface which
                            should be used to load the object.
      @param  fullPath      Full path leading to the data object.
  */
  SmartDataLocator(IDataProviderSvc* pService, const std::string& fullPath)
    : SmartDataStorePtr<TYPE,SmartDataObjectPtr::ObjectLoader>( pService,0,fullPath)
  {
  }
  
  /** Standard constructor: Construct an SmartDataLocator instance which is 
                            able to connect to a DataObject instance
                            which is identified by its directory entry.
                            *** FASTEST ACCESS TO THE DATA STORE ***
      @param  pService      Pointer to the data service interface which
                            should be used to load the object.
      @param  pDirectory    Pointer to the data directory entry. 
  */
  SmartDataLocator(IDataProviderSvc* pService, IRegistry* pDirectory)
    : SmartDataStorePtr<TYPE,SmartDataObjectPtr::ObjectLoader>( pService,pDirectory,"")
  {
  }
  
  /** Standard constructor: Construct an SmartDataLocator instance which is 
                            able to connect to a DataObject instance
                            which is identified by its parent object and
                            the path relative to the parent.
                            The path is meant to address only ONE level,
                            multiple path layers are invalid.
      @param  pService      Pointer to the data service interface which
                            should be used to load the object.
      @param  pObject       Pointer to the parent object.
      @param  path          Path to the data object relative to the parent object.
  */
  SmartDataLocator(IDataProviderSvc* pService, DataObject* pObject, const std::string& path)
    : SmartDataStorePtr<TYPE,SmartDataObjectPtr::ObjectLoader>( pService,0,path)
  {
    if ( 0 != pObject )   {
      this->m_pDirectory = pObject->registry();
    }
  }
  
  /** Standard constructor: Construct an SmartDataLocator instance which is 
                            able to connect to a DataObject instance
                            which is identified by its parent object and
                            the path relative to the parent.
                            The path is meant to address only ONE level,
                            multiple path layers are invalid.
      @param  refObject     Smart Pointer to the parent object.
      @param  pDirectory    Pointer to the data directory entry. 
  */
  SmartDataLocator(SmartDataObjectPtr& refObject, IRegistry* pDirectory)
    : SmartDataStorePtr<TYPE,SmartDataObjectPtr::ObjectLoader>( refObject.service(), pDirectory, "")
  {
  }

  /** Standard constructor: Construct an SmartDataLocator instance which is 
                            able to connect to a DataObject instance
                            which is identified by its parent object and
                            the path relative to the parent.
                            The path is meant to address only ONE level,
                            multiple path layers are invalid.
      @param  refObject     Smart Pointer to the parent object.
      @param  path          Path to the data object relative to the parent object.
  */
  SmartDataLocator(SmartDataObjectPtr& refObject, const std::string& path)
    : SmartDataStorePtr<TYPE,SmartDataObjectPtr::ObjectLoader>( refObject.service(), refObject.directory(), path)
  {
  }

  /** Standard destructor
  */
  virtual ~SmartDataLocator()   {
  }

  /// Automatic conversion to data type
  template <class OTHER>
  SmartDataPtr<OTHER>& operator=( OTHER* pObj )    {
    this->m_pObject = dynamic_cast<TYPE*>(pObj);
    return *this;
  }

  /// Automatic conversion to data type
  template <class OTHER>
  SmartDataPtr<OTHER>& operator=( const OTHER* pObj )    {
    this->m_pObject = dynamic_cast<TYPE*>(const_cast<OTHER*>(pObj));
    return *this;
  }  
};

#endif // GAUDIKERNEL_SMARTDATALOCATOR_H

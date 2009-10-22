//	====================================================================
//	SmartDataStorePtr.h
//	--------------------------------------------------------------------
//
//	Package    : GaudiKernel ( The LHCb Offline System)
//
//  Description: Implementation of a smart pointer class to access
//               easily (and efficiently) data stores.
//
//	Author     : M.Frank
//	====================================================================
#ifndef GAUDIKERNEL_SMARTDATASTOREPTR_H
#define GAUDIKERNEL_SMARTDATASTOREPTR_H 1

// Framework include files
#include "GaudiKernel/SmartDataObjectPtr.h"

/** A small class used to access easily (and efficiently) data items
    residing in data stores.

      The class constructors take several arguments neccessary to be passed
    tyo the data services in order to automatically load objects in case 
    they are not yet loaded. This is achieved through a smart pointer 
    mechanism i.e. by overloading the operator->() at dereferencing time 
    the the object will be requested from the store.

      The SmartDataStorePtr is meant to be "short living". It only makes 
    sense to keep an object instance within e.g. the scope of one method.
    "long living" instances do not make sense and in the contrary
    would be harmful, because the information passed during construction
    to would be invalid and returned object pointers would actually
    point to hyperspace.

      The intrinsic functionality, wether the object will be retrieved or
    loaded from the data store is defined by the LOADER::.


    Base Class:
    SmartDataObjectPtr

    @author  M.Frank
    @version 1.0
*/
template <class TYPE, class LOADER> class SmartDataStorePtr : public SmartDataObjectPtr   {
public:
  /** Standard constructor: Construct an SmartDataStorePtr instance which is 
                            able to connect to a DataObject instance
                            which is identified by its directory entry.
                            *** FASTEST ACCESS TO THE DATA STORE ***
      @param  pService      Pointer to the data service interface which
                            should be used to load the object.
      @param  pDirectory    Pointer to the data directory entry. 
  */
  SmartDataStorePtr(IDataProviderSvc* pService, IRegistry* pRegistry, const std::string& path)
    : SmartDataObjectPtr(LOADER::access(),pService,pRegistry,path), m_pObject(0)
  {
  }
  /** Copy constructor: Construct an copy of a SmartDataStorePtr instance.
      @param  copy          Copy of Smart Pointer to object.
  */
  SmartDataStorePtr(const SmartDataObjectPtr& copy)
    : SmartDataObjectPtr( copy ), m_pObject(0)
  {
  }
  
  /// Standard Destructor
  virtual ~SmartDataStorePtr()  {
  }
  
  /// Automatic conversion to data type
  SmartDataStorePtr& operator=( DataObject* pObj )    {
    m_pObject = dynamic_cast<TYPE*>(pObj);
    return *this;
  }

  /// Automatic conversion to data type
  SmartDataStorePtr& operator=( const DataObject* pObj )    {
    m_pObject = dynamic_cast<TYPE*>(const_cast<DataObject*>(pObj));
    return *this;
  }  

  /// unhides assignment operator of base class
  SmartDataStorePtr& operator=( const SmartDataObjectPtr& copy ) {
    this->m_pObject = dynamic_cast<TYPE*>(const_cast<SmartDataObjectPtr*>(&copy));
    return *this;
  }

  /// Automatic conversion to data type
  TYPE* ptr()    {
    return accessTypeSafeData();
  }
  
  /// Dereference operator: the heart of the smart pointer
  TYPE* operator->()    {
    return accessTypeSafeData();
  }
  
  /// Dereference operator: the heart of the smart pointer
  TYPE& operator*()    {
    TYPE* result = accessTypeSafeData();
    return *result;
  }
  
  /// Automatic conversion to data type
  operator TYPE*()    {
    return accessTypeSafeData();
  }
  
  /// Automatic conversion to data type
  operator TYPE&()    {
    TYPE* result = accessTypeSafeData();
    return *result;
  }

  /// operator int for statements like: if ( SmartDataStorePtr&lt;XXX&gt;(...) ) {}
  operator int()   {
    return 0 != accessTypeSafeData();
  }

  /// operator ! for statements like: if ( !SmartDataStorePtr&lt;XXX&gt;(...) ) {}
  bool operator !()   {
    return 0 == accessTypeSafeData();
  }

  /// Internal type safe accessor to data
  TYPE* accessTypeSafeData()     {
    if ( 0 == m_pObject )   {
      m_pObject = dynamic_cast<TYPE*>(accessData());
    }
    return m_pObject;
  }

protected:
  /// Pointer to data store object
  mutable TYPE*     m_pObject;

};

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    SmartDataPtr<MCParticleVector> mctracks    (evt,"/MC/MCParticles");
    if ( mctracks && mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    @param object_1   Smart pointer to object 1
    @param object_2   Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
template <class A, class LDA, class B, class LDB>  
bool operator&& (SmartDataStorePtr<A, LDA>& object_1, SmartDataStorePtr<B, LDB>& object_2)    {
  if ( 0 != object_1.accessTypeSafeData() )    {     // Test existence of the first object
    if ( 0 != object_2.accessTypeSafeData() )    {   // Test existence of the second object
      return true;                                   // Fine: Both objects exist
    }
  }
  return false;                                      // Tough luck: One is missing.
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    bool test = ...
    if ( test && mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    @param test       First boolean to test
    @param object     Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
template <class B, class LDB>  
bool operator&& (bool test, SmartDataStorePtr<B, LDB>& object)    {
  if ( test )    {                                   // Test existence of the first object
    if ( 0 != object.accessTypeSafeData() )    {     // Test existence of the second object
      return true;                                   // Fine: Both objects exist
    }
  }
  return false;                                      // Tough luck: One is missing.
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    bool test = ...
    if ( test && mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    @param object     Smart pointer to second object
    @param test       Second boolean to test
    @return           Boolean indicating existence of both objects
*/
template <class B, class LDB>  
bool operator&& (SmartDataStorePtr<B, LDB>& object, bool test)    {
  if ( test )    {                                   // Test existence of the first object
    if ( 0 != object.accessTypeSafeData() )    {     // Test existence of the second object
      return true;                                   // Fine: Both objects exist
    }
  }
  return false;                                      // Tough luck: One is missing.
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    SmartDataPtr<MCParticleVector> mctracks    (evt,"/MC/MCParticles");
    if ( mctracks || mcvertices )   { ... }
    and tests the existence of at least one objects in the data store.
    The second object will then NOT be loaded. It is assumed that the second
    choice is only an alternative usable in case the first object
    cannot be retrieved.

    @param object_1   Smart pointer to object 1
    @param object_2   Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
template <class A, class LDA, class B, class LDB>  
bool operator|| (SmartDataStorePtr<A, LDA>& object_1, SmartDataStorePtr<B, LDB>& object_2)    {
  if ( 0 != object_1.accessTypeSafeData() )    {     // Test existence of the first object
    return true;
  }
  if ( 0 != object_2.accessTypeSafeData() )    {     // Test existence of the second object
    return true;
  }
  return false;                                      // Tough luck: Both are missing.
}


/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    bool test = ...
    if ( test || mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    @param test       First boolean to test
    @param object     Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
template <class B, class LDB>  
bool operator|| (bool test, SmartDataStorePtr<B, LDB>& object)    {
  if ( test )    {                                 // Test existence of the first object
    return true;
  }
  if ( 0 != object.accessTypeSafeData() )    {     // Test existence of the second object
    return true;                                   // Fine: Both objects exist
  }
  return false;                                    // Tough luck: One is missing.
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    bool test = ...
    if ( test && mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    @param object     Smart pointer to second object
    @param test       Second boolean to test
    @return           Boolean indicating existence of both objects
*/
template <class B, class LDB>  
bool operator|| (SmartDataStorePtr<B, LDB>& object, bool test)    {
  if ( test )    {                                 // Test existence of the first object
    return true;
  }
  if ( 0 != object.accessTypeSafeData() )    {     // Test existence of the second object
    return true;                                   // Fine: Both objects exist
  }
  return false;                                    // Tough luck: One is missing.
}
#endif // GAUDIKERNEL_SMARTDATASTOREPTR_H

// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/SmartDataObjectPtr.h,v 1.7 2007/11/21 09:29:31 marcocle Exp $
#ifndef GAUDIKERNEL_SMARTDATAOBJECTPTR_H
#define GAUDIKERNEL_SMARTDATAOBJECTPTR_H 1

// STL include files
#include <string>

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"

// Forward declarations
class SmartDataObjectPtr;
class IDataProviderSvc;
class IRegistry;
class DataObject;

/** @class SmartDataObjectPtr SmartDataObjectPtr.h GaudiKernel/SmartDataObjectPtr.h

    A small class used to access easily (and efficiently) data items
    residing in data stores.

      The class is meant as configurable base class for real
    Smart pointer instances. Here mainly the access of the data store
    is handled.
      It is important to keep as less functions as possible NON-VIRTUAL
    in particular those, which handle the data access - they might be called
    very often and hence the compiler must be able to inline them.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API SmartDataObjectPtr {
public:
  typedef DataObject* (* AccessFunction) (SmartDataObjectPtr* ptr);
  /// Helper class to configure smart pointer functionality
  class ObjectLoader    {
  public:
    static AccessFunction access();
  };

  /// Helper class to configure smart pointer functionality
  class ObjectFinder    {
  public:
    static AccessFunction access();
  };

public:
  /** Standard constructor: Construct an SmartDataObjectPtr instance.
      @param  svc          Pointer to the data service
                           interface used to interact with the store.
      @param  pDir         Pointer to data directory
      @param  path         path to object relative to data directory
  */
  SmartDataObjectPtr(AccessFunction access, IDataProviderSvc* pService, IRegistry* pDir, const std::string& path)
    : m_dataProvider(pService),
      m_pRegistry(pDir),
      m_status(StatusCode::SUCCESS,true),
      m_path(path),
      m_accessFunc(access)
  {
  }
  /** Copy constructor: Construct an copy of a SmartDataStorePtr instance.
      @param  copy          Copy of Smart Pointer to object.
  */
  SmartDataObjectPtr(const SmartDataObjectPtr& copy)
    : m_dataProvider(copy.m_dataProvider),
      m_pRegistry(copy.m_pRegistry),
      m_status(copy.m_status),
      m_path(copy.m_path),
      m_accessFunc(copy.m_accessFunc)
  {
  }
  /// Standard Destructor
  virtual ~SmartDataObjectPtr()  {
  }
  /// Assignment operator
  virtual SmartDataObjectPtr& operator=(const SmartDataObjectPtr& copy);

  /// Automatic conversion to data directory
  operator IRegistry*()    {
    return m_pRegistry;
  }

  /// Path name
  const std::string& path()   const   {
    return m_path;
  }

  /// Access to data directory
  IRegistry* directory()    {
    return m_pRegistry;
  }

  /// Assign data service
  void setService( IDataProviderSvc* svc )    {
    m_dataProvider = svc;
  }

  /// Retrieve data service
  IDataProviderSvc* service()    {
    return m_dataProvider;
  }

  /// Access to potential errors during data accesses
  StatusCode getLastError()   const   {
    return m_status;
  }

  /// Static Object retrieval method: must call specific function
  DataObject* accessData()    {
    return m_accessFunc(this);
  }

  /// Static Object retrieval method.
  static DataObject* retrieve(SmartDataObjectPtr* ptr)   {
    return ptr->retrieveObject();
  }

  /// Static Object find method.
  static DataObject* find(SmartDataObjectPtr* ptr)   {
    return ptr->findObject();
  }

  /// Static Object update method.
  static DataObject* update(SmartDataObjectPtr* ptr)   {
    return ptr->updateObject();
  }

  /** Object retrieve method.
     If the object is not known to the local object, it is requested
     from the data service either using the full path if there is no
     directory information present.
  */
  DataObject* retrieveObject();

  /** Object find method.
     If the object is not known to the local object, it is requested
     from the data service either using the full path if there is no
     directory information present.
  */
  DataObject* findObject();

  /** Object update method.
      If the object is not known to the local object, it is requested
      from the data service either using the full path if there is no
      directory information present.
      Needs to be virtual to to implicit object access.
  */
  DataObject* updateObject();

protected:

  /** Find the specified object from the data store.
      @param  pDirectory   Pointer to the directory entry holding the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode find(IRegistry* pDirectory, const std::string& path, DataObject*& refpObject);

  /** Find the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode find(const std::string& fullPath, DataObject*& refpObject);

  /** Retrieve the specified object from the data store.
      @param  pDirectory   Pointer to the directory entry holding the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode retrieve(IRegistry* pDirectory, const std::string& path, DataObject*& refpObject);
  /** Retrieve the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode retrieve(const std::string& fullPath, DataObject*& refpObject);

  /** Update the specified object from the data store.
      @param  pDirectory   Pointer to the directory entry holding the object.
      @return              StatusCode indicating success or failure.
  */
  StatusCode update(IRegistry* pDirectory);
  /** Update the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @return              StatusCode indicating success or failure.
  */
  StatusCode update(const std::string& fullPath);

protected:
  /// Pointer to contained object
  mutable IDataProviderSvc* m_dataProvider;
  /// Pointer to the data registry containing the object
  mutable IRegistry*        m_pRegistry;
  /// Keep track of the last error
  mutable StatusCode        m_status;
  /// Path to object
  std::string               m_path;
  /// Data access function
  AccessFunction            m_accessFunc;

};
#endif // GAUDIKERNEL_SMARTDATAOBJECTPTR_H

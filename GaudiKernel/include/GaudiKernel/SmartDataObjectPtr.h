/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
  using AccessFunction = DataObject* (*)( SmartDataObjectPtr* ptr );
  /// Helper class to configure smart pointer functionality
  class ObjectLoader {
  public:
    static AccessFunction access();
  };

  /// Helper class to configure smart pointer functionality
  class ObjectFinder {
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
  SmartDataObjectPtr( AccessFunction access, IDataProviderSvc* pService, IRegistry* pDir, std::string path )
      : m_dataProvider( pService ), m_pRegistry( pDir ), m_path( std::move( path ) ), m_accessFunc( access ) {}
  /** Copy constructor: Construct an copy of a SmartDataStorePtr instance.
      @param  copy          Copy of Smart Pointer to object.
  */
  SmartDataObjectPtr( const SmartDataObjectPtr& ) = default;

  /// Standard Destructor
  virtual ~SmartDataObjectPtr() = default;

  /// Assignment operator
  virtual SmartDataObjectPtr& operator=( const SmartDataObjectPtr& );

  /// Automatic conversion to data directory
  operator IRegistry*() { return m_pRegistry; }

  /// Path name
  const std::string& path() const { return m_path; }

  /// Access to data directory
  IRegistry* directory() { return m_pRegistry; }

  /// Assign data service
  void setService( IDataProviderSvc* svc ) { m_dataProvider = svc; }

  /// Retrieve data service
  IDataProviderSvc* service() { return m_dataProvider; }

  /// Access to potential errors during data accesses
  const StatusCode& getLastError() const { return m_status; }

  /// Static Object retrieval method: must call specific function
  DataObject* accessData() { return m_accessFunc( this ); }

  /// Static Object retrieval method.
  static DataObject* retrieve( SmartDataObjectPtr* ptr ) { return ptr->retrieveObject(); }

  /// Static Object find method.
  static DataObject* find( SmartDataObjectPtr* ptr ) { return ptr->findObject(); }

  /// Static Object update method.
  static DataObject* update( SmartDataObjectPtr* ptr ) { return ptr->updateObject(); }

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
  StatusCode find( IRegistry* pDirectory, std::string_view path, DataObject*& refpObject );

  /** Find the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode find( std::string_view fullPath, DataObject*& refpObject );

  /** Retrieve the specified object from the data store.
      @param  pDirectory   Pointer to the directory entry holding the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode retrieve( IRegistry* pDirectory, std::string_view path, DataObject*& refpObject );
  /** Retrieve the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @param  refpObject   Reference to the pointer finally holding the object
      @return              StatusCode indicating success or failure.
  */
  StatusCode retrieve( std::string_view fullPath, DataObject*& refpObject );

  /** Update the specified object from the data store.
      @param  pDirectory   Pointer to the directory entry holding the object.
      @return              StatusCode indicating success or failure.
  */
  StatusCode update( IRegistry* pDirectory );
  /** Update the specified object from the data store.
      @param  fullPath     String containing the full path necessary to locate the object.
      @return              StatusCode indicating success or failure.
  */
  StatusCode update( std::string_view fullPath );

protected:
  /// Pointer to contained object
  mutable IDataProviderSvc* m_dataProvider = nullptr;
  /// Pointer to the data registry containing the object
  mutable IRegistry* m_pRegistry = nullptr;
  /// Keep track of the last error
  mutable StatusCode m_status = {StatusCode::SUCCESS, true};
  /// Path to object
  std::string m_path;
  /// Data access function
  AccessFunction m_accessFunc;
};
#endif // GAUDIKERNEL_SMARTDATAOBJECTPTR_H

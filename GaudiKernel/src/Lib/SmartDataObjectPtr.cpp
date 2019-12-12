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
//	====================================================================
//	SmartDataObjectPtr.cpp
//	--------------------------------------------------------------------
//
//	Package   :
//
//	Author    : Markus Frank
//
//	====================================================================
#define GAUDIKERNEL_SMARTDATAOBJECTPTR_CPP 1

// Framework include files
#include "GaudiKernel/SmartDataObjectPtr.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"

SmartDataObjectPtr::AccessFunction SmartDataObjectPtr::ObjectLoader::access() { return &SmartDataObjectPtr::retrieve; }

SmartDataObjectPtr::AccessFunction SmartDataObjectPtr::ObjectFinder::access() { return &SmartDataObjectPtr::find; }

/// Assignment operator
SmartDataObjectPtr& SmartDataObjectPtr::operator=( const SmartDataObjectPtr& copy ) {
  m_path         = copy.m_path;
  m_pRegistry    = copy.m_pRegistry;
  m_dataProvider = copy.m_dataProvider;
  return *this;
}

/// Retrieve the object from the data store.
StatusCode SmartDataObjectPtr::retrieve( IRegistry* pRegistry, const std::string& path, DataObject*& refpObject ) {
  return ( m_dataProvider && pRegistry ) ? m_dataProvider->retrieveObject( pRegistry, path, refpObject )
                                         : StatusCode::FAILURE;
}

/// Retrieve the object from the data store.
StatusCode SmartDataObjectPtr::retrieve( const std::string& fullPath, DataObject*& refpObject ) {
  return m_dataProvider ? m_dataProvider->retrieveObject( fullPath, refpObject ) : StatusCode::FAILURE;
}

/// Retrieve the object from the data store.
StatusCode SmartDataObjectPtr::find( IRegistry* pDirectory, const std::string& path, DataObject*& refpObject ) {
  return ( m_dataProvider && pDirectory ) ? m_dataProvider->findObject( pDirectory, path, refpObject )
                                          : StatusCode::FAILURE;
  ;
}

/// Retrieve the object from the data store.
StatusCode SmartDataObjectPtr::find( const std::string& fullPath, DataObject*& refpObject ) {
  return m_dataProvider ? m_dataProvider->findObject( fullPath, refpObject ) : StatusCode::FAILURE;
}

/// update the object from the data store.
StatusCode SmartDataObjectPtr::update( IRegistry* pRegistry ) {
  return ( m_dataProvider && pRegistry ) ? m_dataProvider->updateObject( pRegistry ) : StatusCode::FAILURE;
}

/// update the object from the data store.
StatusCode SmartDataObjectPtr::update( const std::string& fullPath ) {
  return ( m_dataProvider ) ? m_dataProvider->updateObject( fullPath ) : StatusCode::FAILURE;
}

/** Object retrieval method.
  If the object is not known to the local object, it is requested
  from the data service either using the full path if there is no
  directory information present.
*/
DataObject* SmartDataObjectPtr::retrieveObject() {
  DataObject* pObj = nullptr;
  m_status         = ( !m_pRegistry ? retrieve( m_path, pObj ) : retrieve( m_pRegistry, m_path, pObj ) );
  if ( m_status.isSuccess() ) {
    m_pRegistry = pObj->registry();
    m_path.clear();
  }
  return pObj;
}

/** Object find method.
    If the object is not known to the local object, it is requested
    from the data service either using the full path if there is no
    directory information present.
*/
DataObject* SmartDataObjectPtr::findObject() {
  DataObject* pObj = nullptr;
  m_status         = ( m_pRegistry ? find( m_pRegistry, m_path, pObj ) : find( m_path, pObj ) );
  if ( m_status.isSuccess() ) {
    m_pRegistry = pObj->registry();
    m_path.clear();
  }
  return pObj;
}

/** Object update method.
    If the object is not known to the local object, it is requested
    from the data service either using the full path if there is no
    directory information present.
*/
DataObject* SmartDataObjectPtr::updateObject() {
  DataObject* pObj = accessData(); // Have to load AND update if not present.
  if ( m_status.isSuccess() ) {
    m_status = ( !m_pRegistry ? update( m_path ) : update( m_pRegistry ) );
    if ( !m_status.isSuccess() ) pObj = nullptr;
  }
  return pObj;
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartEvtDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    SmartEvtDataPtr<MCParticleVector> mctracks    (evt,"/MC/MCParticles");
    if ( mctracks && mcvertices )   { ... }
    and tests the existence of BOTH objects in the data store.
    This is not done inline due to danger of code explosion.

    @param object_1   Smart pointer to object 1
    @param object_2   Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
bool operator&&( SmartDataObjectPtr& object_1, SmartDataObjectPtr& object_2 ) {
  return object_1.accessData() && object_2.accessData();
}

/** Helper to test Smart data objects efficiently
    This construct allows statements like:
    SmartEvtDataPtr<MCVertexVector>   mcvertices  (evt,"/MC/MCVertices");
    SmartEvtDataPtr<MCParticleVector> mctracks    (evt,"/MC/MCParticles");
    if ( mctracks || mcvertices )   { ... }
    and tests the existence of at least one objects in the data store.
    The second object will then NOT be loaded. It is assumed that the second
    choice is only an alternative usable in case the first object
    cannot be retrieved.

    This is not done inline due to danger of code explosion.

    @param object_1   Smart pointer to object 1
    @param object_2   Smart pointer to second object
    @return           Boolean indicating existence of both objects
*/
bool operator||( SmartDataObjectPtr& object_1, SmartDataObjectPtr& object_2 ) {
  return object_1.accessData() || object_2.accessData();
}

// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/SmartDataObjectPtr.cpp,v 1.5 2003/06/25 15:59:40 mato Exp $
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
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataObjectPtr.h"

SmartDataObjectPtr::AccessFunction SmartDataObjectPtr::ObjectLoader::access()   {
  return &SmartDataObjectPtr::retrieve;
}

SmartDataObjectPtr::AccessFunction SmartDataObjectPtr::ObjectFinder::access()   {
  return &SmartDataObjectPtr::find;
}

/// Assignment operator
SmartDataObjectPtr& SmartDataObjectPtr::operator=(const SmartDataObjectPtr& copy)   {
  m_path          = copy.m_path;
  m_pRegistry     = copy.m_pRegistry;
  m_dataProvider  = copy.m_dataProvider;
  return *this;
}

/// Retrieve the object from the data store. 
StatusCode SmartDataObjectPtr::retrieve(IRegistry* pRegistry, const std::string& path, DataObject*& refpObject)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider && 0 != pRegistry )    {
    status = m_dataProvider->retrieveObject(pRegistry, path, refpObject);
  }
  return status;
}

/// Retrieve the object from the data store. 
StatusCode SmartDataObjectPtr::retrieve(const std::string& fullPath, DataObject*& refpObject)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider )    {
    status = m_dataProvider->retrieveObject(fullPath, refpObject);
  }
  return status;
}

/// Retrieve the object from the data store. 
StatusCode SmartDataObjectPtr::find(IRegistry* pDirectory, const std::string& path, DataObject*& refpObject)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider && 0 != pDirectory )    {
    status = m_dataProvider->findObject(pDirectory, path, refpObject);
  }
  return status;
}

/// Retrieve the object from the data store. 
StatusCode SmartDataObjectPtr::find(const std::string& fullPath, DataObject*& refpObject)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider )    {
    status = m_dataProvider->findObject(fullPath, refpObject);
  }
  return status;
}


/// update the object from the data store. 
StatusCode SmartDataObjectPtr::update(IRegistry* pRegistry)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider && 0 != pRegistry )    {
    status = m_dataProvider->updateObject(pRegistry);
  }
  return status;
}

/// update the object from the data store. 
StatusCode SmartDataObjectPtr::update(const std::string& fullPath)    {
  StatusCode status = StatusCode::FAILURE;
  if ( 0 != m_dataProvider )    {
    status = m_dataProvider->updateObject(fullPath);
  }
  return status;
}

/** Object retrieval method.
  If the object is not known to the local object, it is requested 
  from the data service either using the full path if there is no
  directory information present.
*/
DataObject* SmartDataObjectPtr::retrieveObject()   {
  DataObject* pObj = 0;
  m_status = (0==m_pRegistry) ? retrieve(m_path,pObj) : retrieve(m_pRegistry,m_path,pObj);
  if ( m_status.isSuccess() )   {
    m_pRegistry = pObj->registry();
    m_path = "";
  }
  return pObj;
}

/** Object find method.
    If the object is not known to the local object, it is requested 
    from the data service either using the full path if there is no
    directory information present.
*/
DataObject* SmartDataObjectPtr::findObject()   {
  DataObject* pObj = 0;
  m_status = (0==m_pRegistry) ? find(m_path,pObj) : find(m_pRegistry,m_path,pObj);
  if ( m_status.isSuccess() )   {
    m_pRegistry = pObj->registry();
    m_path = "";
  }
  return pObj;
}

/** Object update method.
    If the object is not known to the local object, it is requested 
    from the data service either using the full path if there is no
    directory information present.
*/
DataObject* SmartDataObjectPtr::updateObject()   {
  DataObject* pObj = accessData();    // Have to load AND update if not present.
  if ( m_status.isSuccess() )   {
    m_status = (0 == m_pRegistry) ? update(m_path) : update(m_pRegistry);
    if ( !m_status.isSuccess() )  pObj = 0;
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
bool operator&& (SmartDataObjectPtr& object_1, SmartDataObjectPtr& object_2)  {
  if ( 0 != object_1.accessData() )    {      // Test existence of the first object
    if ( 0 != object_2.accessData() )    {    // Test existence of the second object
      return true;                            // Fine: Both objects exist
    }
  }
  return false;                               // Tough luck: One is missing.
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
bool operator|| (SmartDataObjectPtr& object_1, SmartDataObjectPtr& object_2)  {
  if ( 0 != object_1.accessData() )    {      // Test existence of the first object
    return true;
  }
  if ( 0 != object_2.accessData() )    {      // Test existence of the second object
    return true;
  }
  return false;                               // Tough luck: Both are missing.
}


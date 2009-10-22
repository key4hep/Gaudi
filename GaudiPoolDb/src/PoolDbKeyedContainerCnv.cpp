// $Id: PoolDbKeyedContainerCnv.cpp,v 1.8 2008/01/17 13:20:51 marcocle Exp $
//====================================================================
//	KeyedContainerCnv implementation
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: Generic Db data converter for container objects
//
//	Author     : M.Frank
//====================================================================
#include "GaudiKernel/ObjectList.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiKernel/IUpdateable.h"
#include "GaudiPoolDb/PoolDbBaseCnv.h"
#include "GaudiPoolDb/PoolDbKeyedContainerCnv.h"
#include "GaudiKernel/CnvFactory.h"
#include "StorageSvc/DbInstanceCount.h"


namespace {
  union ObjectTypes {
    ObjectTypes(DataObject* p) { Object = p; }
    DataObject*                                             Object;
    ObjectContainerBase*                                    Base;
    ObjectList<ContainedObject>*                            List;
    ObjectVector<ContainedObject>*                          Vector;
    KeyedContainer<KeyedObject<long>, Containers::Map>*     KeyMap;
    KeyedContainer<KeyedObject<long>, Containers::HashMap>* KeyHashMap;
    KeyedContainer<KeyedObject<long>, Containers::Array>*   KeyArray;
    StatusCode update(int flag)  {
      IUpdateable* obj = dynamic_cast<IUpdateable*>(Object);
      if ( obj ) {
        return obj->update(flag);
      }
      return StatusCode::FAILURE;
    }
  };
};

/// Standard Constructor
PoolDbKeyedContainerCnv::PoolDbKeyedContainerCnv(long typ, const CLID& clid, ISvcLocator* svc)
: PoolDbBaseCnv( typ, clid, svc )
{
  pool::DbInstanceCount::increment(this);
}

/// Standard Destructor
PoolDbKeyedContainerCnv::~PoolDbKeyedContainerCnv()    {
  pool::DbInstanceCount::decrement(this);
}

/// Update the references of an updated transient object.
StatusCode 
PoolDbKeyedContainerCnv::updateObjRefs(IOpaqueAddress* pAddr, DataObject* pObj)
{
  StatusCode sc = PoolDbBaseCnv::updateObjRefs(pAddr, pObj);
  if ( sc.isSuccess() )  {
    ObjectTypes obj(pObj);
    const CLID id = (m_objGuid.Data1&0xFFFF0000);
    switch(id) {
    case CLID_ObjectList:              /* ObjectList               */
      return StatusCode::SUCCESS;
    case CLID_ObjectVector:            /* ObjectVector             */
      return StatusCode::SUCCESS;
    case CLID_ObjectVector+0x0030000:  /* Keyed object map         */
      obj.KeyMap->configureDirectAccess();
      return StatusCode::SUCCESS;
    case CLID_ObjectVector+0x0040000:  /* Keyed object hashmap     */
      obj.KeyHashMap->configureDirectAccess();
      return StatusCode::SUCCESS;
    case CLID_ObjectVector+0x0050000:  /* Keyed indirection array  */
      obj.KeyArray->configureDirectAccess();
      return StatusCode::SUCCESS;
    case 0:                            /* Any other object         */
      return StatusCode::SUCCESS;
    default:
      return obj.update(0);
    }
    return StatusCode::FAILURE;
  }
  return sc;
}

PLUGINSVC_FACTORY_WITH_ID( PoolDbKeyedContainerCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00030000),
                           IConverter*(long, CLID, ISvcLocator*) );
PLUGINSVC_FACTORY_WITH_ID( PoolDbKeyedContainerCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00040000),
                           IConverter*(long, CLID, ISvcLocator*) );
PLUGINSVC_FACTORY_WITH_ID( PoolDbKeyedContainerCnv, 
                           ConverterID(POOL_StorageType,CLID_Any + CLID_ObjectVector+0x00050000),
                           IConverter*(long, CLID, ISvcLocator*) );
PLUGINSVC_FACTORY_WITH_ID( PoolDbKeyedContainerCnv, 
                           ConverterID(POOL_StorageType,CLID_Any | (1<<31)),
                           IConverter*(long, CLID, ISvcLocator*) );

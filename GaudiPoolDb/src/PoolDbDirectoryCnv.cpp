// $Id: PoolDbDirectoryCnv.cpp,v 1.8 2008/10/27 16:41:33 marcocle Exp $
//------------------------------------------------------------------------------
//
// Implementation of class :  PoolDbDirectoryCnv
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------

// FIXME: Missing in CORAL
#include <algorithm>

#include <memory>

// Include files
#include "GaudiPoolDb/IPoolDbMgr.h"
#include "GaudiPoolDb/PoolDbDirectoryCnv.h"
#include "GaudiPoolDb/PoolDbNTupleDescriptor.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/CnvFactory.h"

#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbObjectCallBack.h"

// Factory declaration

PLUGINSVC_FACTORY_WITH_ID( PoolDbDirectoryCnv, 
                           ConverterID(POOL_StorageType,CLID_StatisticsDirectory),
                           IConverter*(long, CLID, ISvcLocator*) );


// Standard Constructor
PoolDbDirectoryCnv::PoolDbDirectoryCnv (long typ, 
                                        const CLID& clid, 
                                        ISvcLocator* svc) 
: PoolDbStatCnv(typ, clid, svc)
{
}

/// Create transient object from persistent data
StatusCode 
PoolDbDirectoryCnv::createObj(IOpaqueAddress* /* pAddress */, 
                              DataObject*&       refpObject   )
{
  refpObject = new NTuple::Directory();
  return StatusCode::SUCCESS;
}

StatusCode 
PoolDbDirectoryCnv::createRep(DataObject* pObject, 
			      IOpaqueAddress*& /* refpAddress */)
{
  std::string dsc;
  pool::Guid guid(pool::Guid::null());
  guid.Data1 = objType();

  if ( objType() == CLID_StatisticsDirectory )  {
    dsc = "Directory containing statistics results.";
  }
  else if ( objType() == CLID_StatisticsFile )  {
    dsc = "File containing statistics results.";
  }
  else  {
    return StatusCode::FAILURE;
  }
  std::string ident = containerName(pObject->registry());
  std::string path  = fileName(pObject->registry());
  return saveDescription(path, ident, dsc, "", guid, objType(), "UPDATE");
}

// Fill transient object references
StatusCode
PoolDbDirectoryCnv::fillObjRefs(IOpaqueAddress* pAddr, DataObject* pObj)  
{
  return updateObjRefs(pAddr, pObj);
}

StatusCode 
PoolDbDirectoryCnv::updateObjRefs(IOpaqueAddress* pAddr, 
                                 DataObject* pObject)
{
  typedef std::vector<PoolDbNTupleDescriptor*> REFS;
  REFS refs;
  StatusCode status = StatusCode(StatusCode::FAILURE,true);
  MsgStream log(msgSvc(), "PoolDbDatabaseCnv");
  if ( pAddr ) {
    IRegistry* pReg = pAddr->registry();
    if ( pReg )  {
      std::string ident   = pReg->identifier();
      std::string fname   = fileName(pReg);
      std::string cntName = containerName(pReg);
      std::auto_ptr<pool::DbSelect> iter(m_dbMgr->createSelect("*", fname, "GaudiStatisticsDescription"));
      if ( iter.get() )  {
        pool::DbObjectCallBack cb(pool::DbReflex::forTypeName("PoolDbNTupleDescriptor"));
        typedef std::vector<IRegistry*> Leaves;
        pool::Token* token = 0;
        Leaves leaves;
        while( iter->next(token).isSuccess() )  {
          m_dbMgr->read(&cb, *token).ignore();
          PoolDbNTupleDescriptor* ref = (PoolDbNTupleDescriptor*)cb.object();
          std::string s = ref->container.substr(0,cntName.length());
          if ( s == cntName )  {
            if ( ref->container.length() > cntName.length()+1 )  {
              if ( ref->container.find('/',cntName.length()+1) == std::string::npos ) {
                refs.push_back(ref);
                token->release();
                continue;
              }
            }
          }
          delete ref;
          token->release();
        }
        status = m_dataMgr->objectLeaves(pObject, leaves);
        if ( status.isSuccess() )    {
          for(REFS::iterator i = refs.begin(); i != refs.end(); ++i)  {
            REFS::value_type& ref = *i;
            if ( ref )   {
              bool need_to_add = true;
              for(Leaves::iterator j=leaves.begin(); j != leaves.end(); ++j )  {
                std::string curr_leaf = containerName(*j);
                if ( curr_leaf == ref->container )  {
                  need_to_add = false;
                  break;
                }
              }
              if ( need_to_add )  {
                IOpaqueAddress* pA= 0;
                if ( ref->clid == CLID_StatisticsDirectory ||
                     ref->clid == CLID_StatisticsFile      || 
                     ref->clid == CLID_RowWiseTuple        ||
                     ref->clid == CLID_ColumnWiseTuple      )
                {
                  std::string spar[]   = { fname, ref->container};
                  unsigned long ipar[] = { pool::INVALID, pool::INVALID};
                  status = m_dbMgr->createAddress(repSvcType(),
                                                  ref->clid,
                                                  spar,
                                                  ipar,
                                                  pA);
                }
                if ( status.isSuccess() )  {
                  std::string top = topLevel(pReg);
                  std::string leaf_name = top + ref->container.substr(7);
                  status = m_dataMgr->registerAddress(leaf_name, pA);
                  if ( status.isSuccess() )  {
                    continue;
                  }
                  makeError("Failed to register leaves to directory:"+ident,false).ignore();
                  break;
                }
                makeError("Failed to create leave address to directory:"+ident,false).ignore();
                break;
              }
            }
            makeError("Failed to add leaves to directory:"+ident,false).ignore();
            break;
          }
        }
      }
      else  {
        return makeError("Failed to access required tuple data description.",false);
      }
    }
  }
  for(REFS::iterator k = refs.begin(); k != refs.end(); ++k)  {
    if ( *k ) delete (*k);
  }
  return status;
}

/// Converter overrides: Update transient object from persistent data
StatusCode 
PoolDbDirectoryCnv::updateObj(IOpaqueAddress* /* pAddr */,
                              DataObject*     /* pObj */)
{  
  return StatusCode::SUCCESS;
}

/// Converter overrides: Update persistent object representation.
StatusCode 
PoolDbDirectoryCnv::updateRep(IOpaqueAddress* /* pAddr */,
                              DataObject*     /* pObj */)
{
  return StatusCode::SUCCESS;
}

/// Converter overrides: Update references of persistent object representation.
StatusCode 
PoolDbDirectoryCnv::updateRepRefs(IOpaqueAddress* /* pAddr */,
                                  DataObject*     /* pObj */)
{
  return StatusCode::SUCCESS;
}


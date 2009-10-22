// $Id: PoolDbStatCnv.cpp,v 1.6 2008/10/27 16:41:33 marcocle Exp $
//------------------------------------------------------------------------------
//
// Implementation of class :  PoolDbStatCnv
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------

// Include files
#include "GaudiPoolDb/PoolDbStatCnv.h"
#include "GaudiPoolDb/IPoolDbMgr.h"
#include "GaudiPoolDb/PoolDbNTupleDescriptor.h"

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/CnvFactory.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbObjectCallBack.h"

#include <memory>

// Standard Constructor
PoolDbStatCnv::PoolDbStatCnv (long typ, 
                              const CLID& clid, 
                              ISvcLocator* svc) 
: PoolDbBaseCnv(typ, clid, svc)
{
}

// Retrieve the name of the container a given object is placed into
const std::string PoolDbStatCnv::containerName(IRegistry* pReg) const {
  const std::string& path = pReg->identifier();
  long loc = path.find('/',1);
  std::string local = "<local>";
  if ( loc > 0 )  {
    loc = path.find('/',++loc);
    if ( loc > 0 )  {
      local += path.substr(loc,path.length()-loc);
    }
  }
  return local;
}

const std::string PoolDbStatCnv::fileName(IRegistry* pReg) const {
  std::string path = topLevel(pReg);
  DataObject* pObj = 0;
  dataProvider()->retrieveObject(path, pObj);
  if ( pObj )  {
    NTuple::File* fptr = dynamic_cast<NTuple::File*>(pObj);
    if ( fptr )  {
      return fptr->name();
    }
  }
  return "";
}

/// Retrieve the full path of the file a given object is placed into
const std::string PoolDbStatCnv::topLevel(IRegistry* pReg) const
{
  if ( pReg )    {
    std::string path = pReg->identifier();
    size_t idx = path.find('/', 1);
    if ( idx != std::string::npos )  {
      idx = path.find('/', idx+1);
      if ( idx != std::string::npos )  {
        path = path.substr(0, idx);
      }
      return path;
    }
  }
  return "";
}

StatusCode PoolDbStatCnv::saveDescription(const std::string&  path, 
                                          const std::string&  ident, 
                                          const std::string&  desc,
                                          const std::string&  opt,
                                          const pool::Guid&   guid,
                                          const CLID&         clid,
                                          const std::string&  openMode)
{
  ROOT::Reflex::Type cl = pool::DbReflex::forTypeInfo(typeid(PoolDbNTupleDescriptor));
  const pool::DbTypeInfo* info = pool::DbTypeInfo::create(cl.Name(ROOT::Reflex::SCOPED));
  if ( info )  {
    StatusCode status = m_dbMgr->connectOutput(path, openMode);
    if ( status.isSuccess() )  {
      std::auto_ptr<PoolDbNTupleDescriptor> dsc(new PoolDbNTupleDescriptor());
      dsc->description = desc;
      dsc->optional    = opt;
      dsc->container   = ident;
      dsc->guid        = guid.toString();
      dsc->clid        = clid;
      pool::DataCallBack* call=new pool::DbObjectCallBack(cl);
      call->setShape(info);
      call->setObject(dsc.release());
      status = m_dbMgr->markWrite(call,"GaudiStatisticsDescription");
      if ( status.isSuccess() ) {
        status = m_dbMgr->commitOutput(path, true);
      }
    }
    return status;
  }
  return StatusCode::FAILURE;
}

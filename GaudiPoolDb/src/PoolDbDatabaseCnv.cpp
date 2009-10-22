// $Id: PoolDbDatabaseCnv.cpp,v 1.4 2006/11/30 14:50:44 mato Exp $
//------------------------------------------------------------------------------
//
// Implementation of class :  PoolDbDatabaseCnv
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------
// Include files
#include "GaudiPoolDb/PoolDbDatabaseCnv.h"

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/CnvFactory.h"

// Factory declaration
PLUGINSVC_FACTORY_WITH_ID( PoolDbDatabaseCnv,
                           ConverterID(POOL_StorageType,CLID_StatisticsFile),
                           IConverter*(long, CLID, ISvcLocator*) );

/// Standard Constructor
PoolDbDatabaseCnv::PoolDbDatabaseCnv(long typ, const CLID& cl, ISvcLocator* svc)
: PoolDbDirectoryCnv(typ, cl, svc)
{
}

/// Standard destructor
PoolDbDatabaseCnv::~PoolDbDatabaseCnv()   {
}

// Create database object
StatusCode
PoolDbDatabaseCnv::createObj(IOpaqueAddress* pAddr, DataObject*& refpObj)  {
  StatusCode status = StatusCode::FAILURE;
  MsgStream log(msgSvc(), "PoolDbDatabaseCnv");
  if ( pAddr ) {
    IRegistry* pReg = pAddr->registry();
    const unsigned long* ipars = pAddr->ipar();
    const std::string*   spars = pAddr->par();
    char mode = char(ipars[1]);
    pool::Guid guid(pool::Guid::null());
    std::string fname   = spars[0];
    std::string oname   = pReg->name();
    bool recrea = mode == 'R';
    bool create = mode == 'N';
    bool update = mode == 'U';
    bool read   = mode == 'O';
    const CLID& clid = objType();
    status = StatusCode::SUCCESS;
    guid.Data1 = objType();
    std::string cntName = containerName(pReg);
    if ( create ) {
      status = saveDescription(fname, cntName, "File containing statistics results.",
                               "", guid, clid, "NEW");
      if ( status.isSuccess() )  {
        log << MSG::INFO << "Opened NEW Database file:"
            << fname << " as " << oname << endmsg;
      }
    }
    else if ( update )  {
      status = saveDescription(fname, cntName, "File containing statistics results.",
                               "", guid, clid, "UPDATE");
      if ( status.isSuccess() )  {
        log << MSG::INFO << "Connect to existing Database file:"
            << fname << " as " << oname << " for UPDATE" << endmsg;
      }
    }
    else if ( read ) {
      if ( status.isSuccess() )  {
        log << MSG::INFO << "Connect to existing Database file:"
            << fname << " as " << oname << " for READ" << endmsg;
      }
    }
    else if ( recrea )  {
      status = saveDescription(fname, cntName, "File containing statistics results.",
                               "", guid, clid, "RECREATE");
      if ( status.isSuccess() )  {
        log << MSG::INFO << "Recreate Database file:" << fname << " as " << oname << endmsg;
      }
    }
    else  {
      log << MSG::ERROR << "Don't know what to do:" << fname << endmsg;
      status = StatusCode::FAILURE;
    }
    // Now create object
    if ( status.isSuccess() )  {
      NTuple::File* pFile = new NTuple::File(objType(), fname, oname);
      pFile->setOpen(false);
      refpObj = pFile;
    }
  }
  return status;
}


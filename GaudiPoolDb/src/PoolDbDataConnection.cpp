// $Id: PoolDbDataConnection.cpp,v 1.5 2008/10/27 16:41:33 marcocle Exp $
//====================================================================
//	PoolDbDataConnection implementation
//--------------------------------------------------------------------
//
//  Description: Implementation of the POOL data storage
//
//	Author     : M.Frank
//
//====================================================================
// $Id: PoolDbDataConnection.cpp,v 1.5 2008/10/27 16:41:33 marcocle Exp $

// FIXME: missing in CORAL
#include <algorithm>

// Framework include files
#include "GaudiPoolDb/PoolDbDataConnection.h"
#include "GaudiUtils/IIODataManager.h"
#include "StorageSvc/DbInstanceCount.h"
#include "GaudiKernel/strcasecmp.h"
#include <stdexcept>
#include <iostream>

using namespace pool;

static pool::DbInstanceCount::Counter* s_connectionCount = 
  pool::DbInstanceCount::getCounter(typeid(PoolDbDataConnection));

static inline bool isEqualMode(int mode, int flag)  {
  return (mode&flag)==flag;
}

/// Standard constructor
PoolDbDataConnection::PoolDbDataConnection(IInterface* own, const std::string& nam, int typ, int mode, DbDomain& domH)
: IDataConnection(own,nam), m_type(typ), m_mode(mode), m_domH(domH)
{
  s_connectionCount->increment();
  if ( typ == Gaudi::IIODataManager::FID && strncmp(nam.c_str(),"FID:",4) != 0 ) {
    m_name = "FID:"+nam;
  }
}

/// Standard destructor      
PoolDbDataConnection::~PoolDbDataConnection()         {
  s_connectionCount->decrement();
}

/// Open data stream
StatusCode PoolDbDataConnection::i_connect()  {
  DbAccessMode mode = (DbAccessMode)m_mode;
  bool open_existing = isEqualMode(mode,pool::READ)||isEqualMode(mode,pool::UPDATE)||(1<<4);
  m_dbH = DbDatabase(m_domH.find(fid()));
  if ( !m_dbH.isValid() )  {
    std::string true_fid;
    pool::DbStatus sc = m_dbH.open(m_domH, pfn(), fid(), mode);
    if ( sc.isSuccess() && open_existing )  {
      m_dbH.param("FID",true_fid);
      if ( fid() == pfn() ) {
        m_fid = true_fid;
        m_dbH.close();
        sc = m_dbH.open(m_domH, pfn(), fid(), mode);
      }
      else if ( strcasecmp(true_fid.c_str(),fid().c_str()) != 0 )  {
        m_dbH.close();
	throw std::runtime_error("FID mismatch:\n\t\t\t\tFID="+
				 fid()+" (catalog) <> \n\t\t\t\t"+
				 true_fid+" (file) for PFN="+pfn());
        return BAD_DATA_CONNECTION;
      }
    }
    else if ( open_existing ) {
      return BAD_DATA_CONNECTION;
    }
    return sc.isSuccess() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Release data stream
StatusCode PoolDbDataConnection::disconnect()  {
  if ( m_dbH.isValid() )  {
    m_dbH.close();
    m_dbH = DbDatabase(pool::POOL_StorageType);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}
